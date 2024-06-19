#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cstdlib>
#include <ctime>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <cstring>
#include <algorithm>
#include <sstream>
#include <fcntl.h>
const int MAX_CLIENTS = 10;
const int MAX_ATTEMPTS = 10;

struct Game
{
    int id_socket;
    int secretNumber;
    int lowerBound;
    int upperBound;
    int attemptsLeft;
    bool isActive;
};

std::vector<Game> games;
// std::vector<int> clientSockets;
struct Diap
{
    int min_d;
    int max_d;
    int attempts;
};

struct Config
{
    int port;
    int seed;
    std::vector<Diap> ranges; // Вектор диапазонов
};

int readConfig(const char *filename, Config *conf)
{
    FILE *file = fopen(filename, "r");
    if (file == NULL)
    {
        std::cerr << "File not found " << std::endl;
        return -1;
    }
    fscanf(file, "Port %d\n", &conf->port);
    fscanf(file, "Seed %d\n", &conf->seed);

    
    if (conf->port < 1024 || conf->port > 65535)
    {
        std::cerr << "Port must be in range 1024-65535" << std::endl;
        return -1;
    }
    int counter = 0;
    Diap dp;
    dp.min_d = 0; dp.max_d = 0; dp.attempts= 0;
    std::cout << conf->seed << conf->port << std::endl;
    char c;
    while ((c = fgetc(file)) != '\n' && c != EOF)
        ;
    while (counter < 100 && fscanf(file, "%d %d %d", &dp.min_d, &dp.max_d, &dp.attempts)>0)
    {
        std::cout << dp.min_d << dp.max_d << dp.attempts << std::endl;
        if (dp.min_d >= dp.max_d)
        {
            std::cerr << "Min values must be less than max values" << std::endl;
            return -1;
        }
        if (dp.attempts <= 0)
        {
            std::cerr << "attempts values must be greater than 0" << std::endl;
            return -1;
        }
        conf->ranges.push_back(dp);
        counter++;
    }
    fclose(file);

    return 0;
}

void initializeGames()
{
    games.clear();
}

int getRandomNumber(int lowerBound, int upperBound)
{
    return rand() % (upperBound - lowerBound + 1) + lowerBound;
}

int ind_game(int clientSocket)
{
    for (int i = 0; i < games.size(); i++)
    {
        if (games[i].id_socket == clientSocket)
        {
            return i;
        }
    }
    exit(-1);
}

void initializeServer(int &serverSocket, int port)
{
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1)
    {
        std::cerr << "Error: socket creation failed\n";
        exit(EXIT_FAILURE);
    }

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(port);

    if (bind(serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0)
    {
        std::cerr << "Error: bind failed\n";
        exit(EXIT_FAILURE);
    }

    if (listen(serverSocket, MAX_CLIENTS) < 0)
    {
        std::cerr << "Error: listen failed\n";
        exit(EXIT_FAILURE);
    }
}

int acceptClient(int serverSocket, Config conf)
{
    int clientSocket;
    sockaddr_in clientAddr;
    socklen_t clientAddrLen = sizeof(clientAddr);
    clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddr, &clientAddrLen);
    if (clientSocket < 0)
    {
        return -1;
    }
    int ind = getRandomNumber(0, conf.ranges.size() - 1);

    Game newGame;
    newGame.id_socket = clientSocket;
    newGame.lowerBound = conf.ranges[ind].min_d;
    newGame.upperBound = conf.ranges[ind].max_d;
    newGame.secretNumber = getRandomNumber(newGame.lowerBound, newGame.upperBound);
    std::cout << "Загаданное число: " << newGame.secretNumber << std::endl;
    newGame.attemptsLeft = conf.ranges[ind].attempts;
    newGame.isActive = true;
    games.push_back(newGame);

    std::string buffer(1024, '\0'); // Используем конструктор строки для инициализации и задания размера

    int valread = recv(clientSocket, &buffer[0], buffer.size(), 0);
    std::cout << "2" << std::endl;
    if (valread <= 0)
    {
        if (valread < 0)
        {
            std::cerr << "Error: recv failed.\n";
        }
        else
        {
            std::cerr << "Connection closed by client.\n";
        }
        close(clientSocket);
        games.pop_back(); // Удаляем последнюю добавленную игру из вектора, т.к. клиент не присоединился
        return -1;
    }

    buffer.resize(valread);
    int currentPlayerIndex = ind_game(clientSocket);

    std::string welcomeMessage = std::to_string(games[currentPlayerIndex].lowerBound) + " " + std::to_string(games[currentPlayerIndex].upperBound) + " " + std::to_string(games[currentPlayerIndex].attemptsLeft);
    send(clientSocket, welcomeMessage.c_str(), welcomeMessage.length(), 0);
    // std::string message = "Количество доступных попыток:" + std::to_string(games[currentPlayerIndex].attemptsLeft) + "\n";
    // send(clientSocket, message.c_str(), message.length(), 0);
    std::cerr << std::to_string(clientSocket) << " " << buffer << std::endl;
    return clientSocket;
}

void handleClient(int clientSocket)
{
    // std::string playerName;

    // char buffer[1024] = {0};

    std::string buffer(1024, '\0');
    // int valread = recv(clientSocket, buffer, 1024, 0);
    // playerName = buffer;
    int currentPlayerIndex = ind_game(clientSocket);

    int valread = recv(clientSocket, &buffer[0], buffer.size(), 0);
    std::cout << "3" << std::endl;
    if (valread <= 0)
    {
        // Обработка ошибки при чтении данных
        if (valread < 0)
        {
            // Обработка ошибки при чтении данных
            std::cerr << "Error: recv failed." << std::to_string(valread) << std::endl;
        }
        else
        {
            // Соединение закрыто клиентом
            std::cerr << "Connection closed by client.\n";
        }
        close(clientSocket);
        games.erase(games.begin() + currentPlayerIndex);
        return;
    }
    buffer[valread] = '\0';
    std::cout << buffer << " " << games[currentPlayerIndex].secretNumber << std::endl;
    buffer.resize(valread);
    std::string operation;
    std::string value;
    int guessedNumber = 0;
    size_t pos = buffer.find(" ");
    if (pos != std::string::npos)
    {
        operation = buffer.substr(0, pos);
        value = buffer.substr(pos + 1);

        buffer.resize(valread);
    }
    // std::cout << value << "1111";
    guessedNumber = std::stoi(value);
    // std::cout << operation << std::endl;
    if (operation == "меньше")
    {
        if (guessedNumber > games[currentPlayerIndex].secretNumber)
        {
            std::string winMessage = "Угадали1!\n";
            send(clientSocket, winMessage.c_str(), winMessage.length(), 0);
            // send(clientSocket, "1", 1, 0);
            std::cout << winMessage.length() << std::endl;
        }
        else
        {
            std::string loseMessage = "Не угадали2!\n";
            send(clientSocket, loseMessage.c_str(), loseMessage.length(), 0);
            // send(clientSocket, "2", 1, 0);
        }
    }
    else if (operation == "больше")
    {
        if (guessedNumber < games[currentPlayerIndex].secretNumber)
        {
            std::string winMessage = "Угадали3!\n";
            send(clientSocket, winMessage.c_str(), winMessage.length(), 0);
            std::cout << guessedNumber << games[currentPlayerIndex].secretNumber << std::endl;
        }
        else
        {
            std::string loseMessage = "Не угадали4!\n";
            send(clientSocket, loseMessage.c_str(), loseMessage.length(), 0);
        }
    }
    else if (operation == "равно")
    {
        if (guessedNumber == games[currentPlayerIndex].secretNumber)
        {
            std::string winMessage = "Ура! Вы отгадали загаданное число!\n";
            send(clientSocket, winMessage.c_str(), winMessage.length(), 0);
            games[currentPlayerIndex].isActive = false;
        }
        else
        {
            std::string loseMessage = "Не угадали5!\n";
            send(clientSocket, loseMessage.c_str(), loseMessage.length(), 0);
        }
    }
    else
    {
        std::string invalidMessage = "Некорректный ввод. Используйте \"больше\", \"меньше\" или \"равно\".\n";
        send(clientSocket, invalidMessage.c_str(), invalidMessage.length(), 0);
        // continue; // пропускаем остальной код цикла
    }
    if (games[currentPlayerIndex].isActive)
        games[currentPlayerIndex].attemptsLeft--;
    if (games[currentPlayerIndex].attemptsLeft == 0 && games[currentPlayerIndex].isActive)
    {
        // std::string loseMessage = "Попытки закончились. Загаданное число - " +
        //                           std::to_string(games[currentPlayerIndex].secretNumber) + ".\n";
        // send(clientSocket, loseMessage.c_str(), loseMessage.length(), 0);
        games[currentPlayerIndex].isActive = false;
    }
    else if (games[currentPlayerIndex].isActive == false)
    {
        //     std::string message = "Количество оставшихся попыток:" + std::to_string(games[currentPlayerIndex].attemptsLeft) + "\n";
        //     send(clientSocket, message.c_str(), message.length(), 0);
        close(clientSocket);
        games.erase(games.begin() + currentPlayerIndex);
    }
    // else
    // {
    // std::string message = "Количество доступных попыток:" + std::to_string(games[currentPlayerIndex].attemptsLeft) + "\n";
    // send(clientSocket, message.c_str(), message.length(), 0);
    // }
}

int main()
{
    const char *filename = "conf.txt"; // Путь к вашему файлу конфигурации
    Config config;
    if (readConfig(filename, &config)==-1)
    {

        return -1;
    }
    srand(config.seed);
    int serverSocket;
    initializeServer(serverSocket, config.port);
    initializeGames();
    fcntl(serverSocket, F_SETFL, O_NONBLOCK);
    while (true)
    {
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(serverSocket, &readfds);

        int maxSocket = serverSocket;
        for (int i = 0; i < games.size(); ++i)
        {
            FD_SET(games[i].id_socket, &readfds);
            if (games[i].id_socket > maxSocket)
                maxSocket = games[i].id_socket;
        }

        int activity = select(maxSocket + 1, &readfds, NULL, NULL, NULL);
        if ((activity < 0) && (errno != EINTR))
        {
            std::cerr << "Error: select error\n";
            exit(EXIT_FAILURE);
        }

        if (FD_ISSET(serverSocket, &readfds))
        {
            int clientSocket = acceptClient(serverSocket, config);
            if (clientSocket < 0)
            {
                std::cerr << "Error: acceptClient error\n";
                continue;
            }
            fcntl(clientSocket, F_SETFL, O_NONBLOCK);
        }

        for (int i = 0; i < games.size(); ++i)
        {
            int clientSocket = games[i].id_socket;
            if (FD_ISSET(clientSocket, &readfds))
            {
                handleClient(clientSocket);
            }
        }
    }

    close(serverSocket);

    return 0;
}