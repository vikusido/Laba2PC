#include <iostream>   //для ввода/вывода
#include <cstring>    //для работы со строками
#include <fcntl.h>    //для работы с файловыми дескрипторами
#include <sys/stat.h> //для создания FIFO
#include <unistd.h>   //для системных вызовов (read, write, close)
#include <signal.h>   //для обработки сигнала
#include <errno.h>    //для обработки ошибок

using namespace std;

// преобразование пути в PATH
#define PATH "/tmp/pingpong_fifo"

// обработка сигнала SIGINT (ctrl+C)
void handle_sigint(int sig)
{
    cout << "\nThe server is shutting down. Removing FIFO..." << endl;
    unlink(PATH);
    exit(0);
}

int main()
{
    // установка обработчика сигнала SIGINT
    signal(SIGINT, handle_sigint);

    // создаем FIFO, если его не существует
    if (mkfifo(PATH, 0666) == -1 && errno != EEXIST)
    {
        perror("Error creating FIFO");
        return 1;
    }

    cout << "Server is running... Waiting for the client's messages." << endl;

    while (true)
    {
        // открываем FIFO для чтения и записываем файловый дескриптор в fd
        int fd = open(PATH, O_RDONLY);
        // обработка ошибки открытия FIFO
        if (fd == -1)
        {
            perror("Error opening FIFO for reading");
            return 1;
        }

        // массив для хранения считанных данных
        char buffer[1024] = {0};
        ssize_t bytes_read = read(fd, buffer, sizeof(buffer) - 1);

        if (bytes_read > 0)
        {
            // добавляем символ конца строки и выводим сообщение
            buffer[bytes_read] = '\0';
            cout << "Received: " << buffer << endl;

            // проверка не завершил ли работу пользователь
            if (strcmp(buffer, "exit") == 0)
            {
                cout << "Client requested to exit. Shutting down server." << endl;
                close(fd);
                break;
            }

            // открываем FIFO для записи
            fd = open(PATH, O_WRONLY);
            if (fd == -1)
            {
                perror("Error opening FIFO for writing");
                close(fd);
                return 1;
            }
            // формируем строку с ответом и записываем её
            string response = "Wow, " + string(buffer) + "!";
            write(fd, response.c_str(), response.length());
            close(fd);
        }
        // обработка случаев, когда bytes_read не положителен
        else if (bytes_read == -1)
        {
            perror("Error reading from FIFO");
            close(fd);
        }
        else if (bytes_read == 0)
        {
            cout << "EOF or empty FIFO" << endl;
            close(fd);
        }
    }

    unlink(PATH); // yдаляем FIFO при завершении
    return 0;
}