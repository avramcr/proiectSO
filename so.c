#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

typedef struct {
    uint16_t signature;   
    uint32_t file_size;    
    uint32_t reserved;
    uint32_t data_offset; 
} Header;
typedef struct {
    uint32_t size; // size headerinfo=40;
    uint32_t width;
    uint32_t height;
    uint16_t planes;
    uint16_t bit_count; 
    uint32_t compression;
    uint32_t image_size;
    uint32_t x_pixels_per_m;
    uint32_t y_pixels_per_m;
    uint32_t colors_used;
    uint32_t colors_important;
} HeaderInfo;

int main(int argc, char *argv[]) {
    if (argc != 2) {
        perror("Număr incorect de argumente");
        exit(EXIT_FAILURE);
    }

    int input_destination = open(argv[1], O_RDONLY);
    if (input_destination == -1) {
        perror("Eroare deschidere fisier");
        exit(EXIT_FAILURE);
    }

    struct stat file_info;
    if (stat(argv[1], &file_info) == -1) {
        perror("Eroare la primire informatii");
        close(input_destination);
        exit(EXIT_FAILURE);
    }

    Header header;
    ssize_t citeste = read(input_destination, &header, sizeof(Header));
    if (citeste == -1) {
        perror("Eroare citire fisier header");
        close(input_destination);
       exit(EXIT_FAILURE);
    }

    HeaderInfo info;
    citeste = read(input_destination, &info, sizeof(HeaderInfo));
    if (citeste == -1) {
        perror("Eroare la citire fisier info");
        close(input_destination);
        exit(EXIT_FAILURE);
    }

    off_t total_size = file_info.st_size;
    uid_t owner = file_info.st_uid;
    time_t modificare = file_info.st_mtim.tv_sec;
    nlink_t legaturi = file_info.st_nlink;

    char user[4];

    if (file_info.st_mode & S_IRUSR) {
        user[0] = 'R';
    } else {
        user[0] = '-';
    }

    if (file_info.st_mode & S_IWUSR) {
        user[1] = 'W';
    } else {
        user[1] = '-';
    }

    if (file_info.st_mode & S_IXUSR) {
        user[2] = 'X';
    } else {
        user[2] = '-';
    }

   user[3] = '\0';
   char grup[4];

    if (file_info.st_mode & S_IRGRP) {
        grup[0] = 'R';
    } else {
        grup[0] = '-';
    }

    if (file_info.st_mode & S_IWGRP) {
        grup[1] = 'W';
    } else {
       grup[1] = '-';
    }

    if (file_info.st_mode & S_IXGRP) {
        grup[2] = 'X';
    } else {
        grup[2] = '-';
    }
    grup[3] = '\0';
    char altii[4];

    if (file_info.st_mode & S_IROTH) {
       altii[0] = 'R';
    } else {
        altii[0] = '-';
    }

    if (file_info.st_mode & S_IWOTH) {
        altii[1] = 'W';
    } else {
        altii[1] = '-';
    }

    if (file_info.st_mode & S_IXOTH) {
        altii[2] = 'X';
    } else {
        altii[2] = '-';
    }
    altii[3] = '\0';
    

    char buffer[4096];
    sprintf(buffer, "Numele pozei este: %s\nInaltimea este: %d\nLungimea este: %d\nDimensiunea totala a fisierului este: %ld\nIdentificatorul utilizatorului: %d\nUltima modificare: %sNumarul de legaturi este: %ld\nDrepturile de acces ale owner-ului: %s\nDrepturile de acces ale grupului: %s\nDrepturile de acces altii: %s\n",
            argv[1], info.height, info.width, total_size, owner, ctime(&modificare), legaturi,user,grup,altii);

    int output_destination = open("statistica.txt", O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    if (output_destination == -1) {
        perror("Eroare la deschiderea fisierului de output");
        close(input_destination);
        exit(EXIT_FAILURE);
    }

    ssize_t scrie = write(output_destination, buffer, strlen(buffer));
    if (scrie == -1) {
        perror("Eroare la scriere in fisier"); 
        close(output_destination);
         exit(EXIT_FAILURE);
    }

    close(output_destination);
    close(input_destination);

    return 0;
}
