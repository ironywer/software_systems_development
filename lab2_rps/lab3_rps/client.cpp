#include <iostream>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <cstring>
#include <cctype>
#define sizeofbuffer 2056

bool sisDigit(std::string strk){
    char sym = 0;
    int i = 0;
    while ((sym = strk[i])!= '\0'){
        if (!isdigit(sym)){
            return false;
        }
        i++;
        std::cout<<i;
    }
    return true;
}

int main()
{
    int clientSocket;
    struct sockaddr_in serverAddr;
    // std::string buffer(1024, '\0');
    char buffer2[sizeofbuffer] = {0};
    if ((clientSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        std::cerr << "Socket creation error\n";
        return -1;
    }

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(8080);

    if (inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr) <= 0)
    {
        std::cerr << "Invalid address\n";
        return -1;
    }

    if (connect(clientSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0)
    {
        std::cerr << "Connection failed\n";
        return -1;
    }

    std::string playerName;
    std::cout << "Введите имя: ";
    std::cin >> playerName;

    send(clientSocket, playerName.c_str(), playerName.length(), 0);
    // // Ждем 1 секунду перед вызовом recv
    // usleep(1000000);

    int bytesReceived = recv(clientSocket, buffer2, sizeofbuffer, 0);
    if (bytesReceived < 0)
    {
        std::cerr << "Failed to receive data from server\n";
        return -1;
    }
    else if (bytesReceived == 0)
    {
        std::cerr << "Connection closed by server\n";
        return -1;
    }
    int min_d = 0, max_d = 0, attempts = 0;
    sscanf(buffer2, "%d %d %d", &min_d, &max_d, &attempts);
    std::cout << "Добро пожаловать в игру, " + playerName + "!\nВам необходимо отгадать число в диапазоне от " + std::to_string(min_d) +
                                 " до " + std::to_string(max_d) + ", используя следующие операторы \"больше\", \"меньше\" или \"равно\"\n" + "Количество доступных попыток:" + std::to_string(attempts) + "\n";
    // buffer.clear();
    while (true)
    {
        std::cout << "Ваше предположение: X ";

        std::string input_oper;
        std::string input_num;
        std::cin >> input_oper >> input_num;

        std::cin.clear();
        std::cin.sync();
        // std::cout << input_oper << input_num << std::endl;
        if (input_oper != "больше" && input_oper != "меньше" && input_oper != "равно" || sisDigit(input_num)==false){
            std::cout << "Некорректный ввод. Используйте \"больше\", \"меньше\" или \"равно\"." << std::endl;
            continue;
        }
        std::string input_op_n = input_oper + ' ' + input_num;
        send(clientSocket, input_op_n.c_str(), input_op_n.length(), 0);
        // send(clientSocket, "больше 50", 50, 0);
        int bytesReceived = recv(clientSocket, buffer2, sizeofbuffer, 0);
        if (bytesReceived < 0)
        {
            std::cerr << "Failed to receive data from server\n";
            break;
        }
        else if (bytesReceived == 0)
        {
            std::cerr << "Connection closed by server1\n";
            // std::cout << buffer.size() << std::endl;
            break;
        }
        std::cout << bytesReceived << std::endl;
        printf("%s", buffer2);
        // buffer.clear();

        // bytesReceived = recv(clientSocket, buffer2, sizeofbuffer, 0);
        // if (bytesReceived < 0)
        // {
        //     std::cerr << "Failed to receive data from server\n";
        //     break;
        // }
        // else if (bytesReceived == 0)
        // {
        //     std::cerr << "Connection closed by server2\n";
        //     break;
        // }

        // std::cout << buffer2;
        attempts--;
        if (strstr(buffer2, "отгадали") != NULL)
        {
            std::cout << "Конец игры. Попыток осталось "<< std::to_string(attempts) << std::endl;
            break;
        }
        else if (attempts == 0){
            std::cout << "Конец игры. Попытки закончились" << std::endl;
            break;
        }
        else{
            std::cout << "Доступно попыток: "<< std::to_string(attempts) << std::endl;
        }

        // buffer.clear();
    }

    close(clientSocket);
    return 0;
}
