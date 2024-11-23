#include <iostream>   //для ввода/вывода
#include <fcntl.h>    //для работы с файловыми дескрипторами
#include <unistd.h>   //для системных вызовов (read, write, close)
#include <sys/stat.h> //для создания FIFO
#include <errno.h>    //для обработки ошибок

using namespace std;

// преобразование пути в PATH
#define PATH "/tmp/pingpong_fifo"

int main()
{
    cout << "Client is running... Type 'exit' to quit." << endl;
    // флаг для отслеживания существования FIFO
    bool FIFO_is_found = true;

    while (true)
    {
        // проверка на существование FIFO
        if (access(PATH, F_OK) == -1)
        {
            if (FIFO_is_found)
            {
                cout << "FIFO does not exist. Is the server running?" << endl;
                FIFO_is_found = false;
            }
            sleep(1); // пауза между проверками
            continue;
        }
        else
        {
            FIFO_is_found = true;
        }

        // запись в message ввода пользователя
        string message;
        cout << "Enter message (ping): ";
        getline(cin, message);

        // открываем FIFO для записи и записываем файловый дескриптор в fd
        int fd = open(PATH, O_WRONLY);
        // обработка ошибки открытия FIFO
        if (fd == -1)
        {
            perror("Error opening FIFO for writing");
            return 1;
        }
        // записвыаем ввод пользователя
        write(fd, message.c_str(), message.length());
        close(fd);

        // проверяем не завершает ли работу пользователь
        if (message == "exit")
        {
            cout << "Exit." << endl;
            break;
        }

        // открываем FIFO для чтения
        fd = open(PATH, O_RDONLY);
        // обработка ошибки открытия FIFO
        if (fd == -1)
        {
            perror("Error opening FIFO for reading");
            return 1;
        }

        // массив для хранения считанных данных
        char buffer[1024] = {0};
        ssize_t bytes_read = read(fd, buffer, sizeof(buffer) - 1);
        close(fd);

        if (bytes_read > 0)
        {
            // добавляем символ конца строки и выводим сообщение
            buffer[bytes_read] = '\0';
            cout << "Server response: " << buffer << endl;
        }
        // обработка случаев, когда bytes_read не положителен
        else if (bytes_read == -1)
        {
            perror("Error reading from FIFO");
        }
        else if (bytes_read == 0)
        {
            cout << "EOF or empty FIFO" << endl;
        }
    }

    return 0;
}
