Se va scrie un program în limbajul C ce va prelucra un fișier de intrare ce reprezinta o imagine in format BMP și va realiza o serie de statistici pe baza acestui fișier. Programul
va primi un parametru și se va apela după cum urmează: ./program <fisier_intrare>
Programul va verifica faptul că a primit un singur argument, precum și tipul acestuia, iar în caz contrar va afișa un mesaj de eroare ”Usage ./program <fisier_intrare>”.
Programul trebuie sa citeasca header-ul fisierului BMP și sa extraga valoarea înălțimii,respectiv a lungimii pentru imaginea data.
Programul va crea un fișier cu numele statistica.txt în care vor fi scrise următoarele informații:
nume fisier: poza.bmp
inaltime: 1920
lungime: 1280
dimensiune: <dimensiune in octeti>
identificatorul utilizatorului: <user id>
timpul ultimei modificari: 28.10.2023
contorul de legaturi: <numar legaturi>
drepturi de acces user: RWX
drepturi de acces grup: R–-
drepturi de acces altii: ---
Se vor folosi doar apeluri sistem pentru lucrul cu fișierele (open, read, write, close, stat,
fstat, lstat... etc)



#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <time.h>
#include <sys/wait.h>
 
typedef struct {
    uint16_t signature;
    uint32_t file_size;
    uint32_t reserved;
    uint32_t data_offset;
} Header; 
 
typedef struct {
    uint32_t size;
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
typedef struct {
    uint8_t red;
    uint8_t green;
    uint8_t blue;
} Pixel;
void convertire(const char *caleFisier,char c) {
    int fisier = open(caleFisier, O_RDWR);
    if (fisier == -1) {
        perror("Eroare deschidere fisier");
        exit(EXIT_FAILURE);
    } 

    Header header;
    ssize_t citeste = read(fisier, &header, sizeof(Header) - 2);
    if (citeste == -1) {
        perror("Eroare citire fisier header");
        close(fisier);
        exit(EXIT_FAILURE);
    }

    HeaderInfo info;
    citeste = read(fisier, &info, sizeof(HeaderInfo));
    if (citeste == -1) {
        perror("Eroare la citire fisier info");
        close(fisier);
        exit(EXIT_FAILURE);
    }

    Pixel pixel;
    for (uint32_t i = 0; i < info.height; i++) {
        for (uint32_t j = 0; j < info.width; j++) {
            citeste = read(fisier, &pixel, sizeof(Pixel));
            if (citeste == -1) {
                perror("Eroare la citire pixel");
                close(fisier);
                exit(EXIT_FAILURE);
            }
 
            uint8_t pixelGray = (uint8_t)(0.299 * pixel.red + 0.587 * pixel.green + 0.114 * pixel.blue);

	    lseek(fisier,-3, SEEK_CUR);
            write(fisier, &pixelGray, sizeof(pixelGray));
            write(fisier, &pixelGray, sizeof(pixelGray));
            write(fisier, &pixelGray, sizeof(pixelGray));
        } 
    } 

    close(fisier); 
}
void continut(int pfd[2],const char *caleFisier)
{
  int input = open(caleFisier, O_RDONLY);
    if (input == -1) {
        perror("Eroare deschidere fisier");
        exit(EXIT_FAILURE);
    }

    char buffer[4096];
    ssize_t citeste;
    while ((citeste = read(input, buffer, sizeof(buffer))) > 0)
      {
        ssize_t scris = write(pfd[1], buffer, citeste);
        if (scris == -1) {
            perror("Eroare la scrierea in pipe");
            exit(EXIT_FAILURE);
        }
    }
  close(input);
  close(pfd[1]);
}
void fisier(const char *caleFisier, const char *directorIesire,char c) {
    int input_destination = open(caleFisier, O_RDONLY);
    if (input_destination == -1) {
        perror("Eroare deschidere fisier");
        exit(EXIT_FAILURE);
    }
 
    struct stat file_info;
    if (stat(caleFisier, &file_info) == -1) {
        perror("Eroare la primire informatii");
        close(input_destination);
        exit(EXIT_FAILURE);
    }
 
    Header header; 
    ssize_t citeste = read(input_destination, &header, sizeof(Header) - 2);
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

    if (strstr(caleFisier, ".bmp") != NULL) {
        pid_t pid;
        pid = fork();

        if (pid == -1) {
            perror("Procesul nu s-a creat");
            exit(EXIT_FAILURE);
        } else if (pid == 0) {
            convertire(caleFisier, c);
        } else {
            int status;
            wait(&status); 
            printf("S-a încheiat procesul cu PID-ul %d și codul %d\n", pid, WEXITSTATUS(status));
        }
	 snprintf(buffer, sizeof(buffer), "Numele pozei este: %s\nInaltimea este: %d\nLungimea este: %d\nDimensiunea totala a fisierului este: %ld\nIdentificatorul utilizatorului: %d\nUltima modificare: %sNumarul de legaturi este: %ld\nDrepturile de acces ale owner-ului: %s\nDrepturile de acces ale grupului: %s\nDrepturile de acces altii: %s\n",
                 caleFisier, info.height, info.width, total_size, owner, ctime(&modificare), legaturi, user, grup, altii);
    } else {
      char proba[4096];
        int pfd[2];
        if (pipe(pfd) == -1) //scriere 1 si citire 0
	  {
            perror("Eroare la crearea pipe-ului");
            exit(EXIT_FAILURE);
        }
        pid_t pid_fiu;
	pid_fiu = fork(); 

        if (pid_fiu == -1) {
            perror("Procesul fiu nu s-a creat");
            exit(EXIT_FAILURE);
        } else if (pid_fiu == 0)
	  {
	    close(pfd[1]);
	    ssize_t citeste=read(pfd[0],proba,strlen(buffer));
	    if(citeste==-1)
	      {
		perror("Eroare la citire pipe");
		  exit(EXIT_FAILURE);
	      }
	    close(pfd[0]);
	    
	  } else {
	    close(pfd[0]);
	    continut(pfd,caleFisier);
	    close(pfd[1]);
	    int status_fiu;
	    waitpid(pid_fiu,&status_fiu,0);
	    printf("S-a încheiat procesul fiu cu PID-ul %d și codul %d\n", pid_fiu, WEXITSTATUS(status_fiu));
        }
	 snprintf(buffer, sizeof(buffer), "Numele fisierului este: %s\nDimensiunea totala a fisierului este: %ld\nIdentificatorul utilizatorului: %d\nUltima modificare: %sNumarul de legaturi este: %ld\nDrepturile de acces ale owner-ului: %s\nDrepturile de acces ale grupului: %s\nDrepturile de acces altii: %s\n",
             caleFisier, total_size, owner, ctime(&modificare), legaturi, user, grup, altii);
    } 

    char fisierulMeuDeStatistica[4096];
    snprintf(fisierulMeuDeStatistica, sizeof(fisierulMeuDeStatistica), "%s/%s_statistica.txt", directorIesire, caleFisier);

    int output_destination = open(fisierulMeuDeStatistica, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
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
}
void director(const char *caleDirector, const char *directorIesire,char c) {
    DIR *dir = opendir(caleDirector);
    if (dir == NULL) {
        perror("Eroare la deschiderea directorului");
        exit(EXIT_FAILURE);
    }
 
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
            char buffer[4096];
            snprintf(buffer, sizeof(buffer), "%s/%s", caleDirector, entry->d_name);
 
            if (entry->d_type == DT_REG) {
                pid_t pid;
                pid = fork();
 
                if (pid == -1) {
                    perror("Procesul nu s-a creat");
                    exit(EXIT_FAILURE);
                } else if (pid == 0) {
		  fisier(buffer, directorIesire,c);
                } else {
                    int status;
                    wait(&status);
                    printf("S-a încheiat procesul cu PID-ul %d și codul %d\n", pid, WEXITSTATUS(status));
                }
            } else if (entry->d_type == DT_DIR) {
                struct stat dir_info;
                if (stat(buffer, &dir_info) == -1) {
                    perror("Eroare la primirea informatii director");
                    closedir(dir);
                    exit(EXIT_FAILURE);
                }
  
                char user[4];
                if (dir_info.st_mode & S_IRUSR) {
                    user[0] = 'R';
                } else {
                    user[0] = '-';
                }
 
                if (dir_info.st_mode & S_IWUSR) {
                    user[1] = 'W';
                } else {
                    user[1] = '-';
                }
 
                if (dir_info.st_mode & S_IXUSR) {
                    user[2] = 'X';
                } else {
                    user[2] = '-';
                }
 
                user[3] = '\0';
 
                char grup[4];
                if (dir_info.st_mode & S_IRGRP) {
                    grup[0] = 'R';
                } else {
                    grup[0] = '-';
                }
 
                if (dir_info.st_mode & S_IWGRP) {
                    grup[1] = 'W';
                } else {
                    grup[1] = '-';
                }
 
                if (dir_info.st_mode & S_IXGRP) {
                    grup[2] = 'X';
                } else {
                    grup[2] = '-';
                }
                grup[3] = '\0';
 
                char altii[4];
                if (dir_info.st_mode & S_IROTH) {
                    altii[0] = 'R';
                } else {
                    altii[0] = '-';
                }
 
                if (dir_info.st_mode & S_IWOTH) {
                    altii[1] = 'W';
                } else {
                    altii[1] = '-';
                }
 
                if (dir_info.st_mode & S_IXOTH) {
                    altii[2] = 'X';
                } else {
                    altii[2] = '-';
                }
                altii[3] = '\0';
 
                snprintf(buffer, sizeof(buffer), "Nume director: %s\nIdentificatorul utilizatorului: %d\nDrepturi de acces user: %s\nDrepturi de acces grup: %s\nDrepturi de acces altii: %s\n", entry->d_name, dir_info.st_uid, user, grup, altii);
                printf("%s", buffer);
                director(buffer, directorIesire,c);
            } else if (entry->d_type == DT_LNK) {
                struct stat info_lnk;
                if (lstat(buffer, &info_lnk) == -1) {
                    perror("Eroare primire informatie link");
                    closedir(dir);
                    exit(EXIT_FAILURE);
                }
 
                char owner[4];
                if (info_lnk.st_mode & S_IRUSR) {
                    owner[0] = 'R';
                } else {
                    owner[0] = '-';
                }
 
                if (info_lnk.st_mode & S_IWUSR) {
                    owner[1] = 'W';
                } else {
                    owner[1] = '-';
                }
 
                if (info_lnk.st_mode & S_IXUSR) {
                    owner[2] = 'X';
                } else {
                    owner[2] = '-';
                }
 
                owner[3] = '\0';
 
                char group[4];
                if (info_lnk.st_mode & S_IRGRP) {
                    group[0] = 'R';
                } else {
                    group[0] = '-';
                }
 
                if (info_lnk.st_mode & S_IWGRP) {
                    group[1] = 'W';
                } else {
                    group[1] = '-';
                }
 
                if (info_lnk.st_mode & S_IXGRP) {
                    group[2] = 'X';
                } else {
                    group[2] = '-';
                }
                group[3] = '\0';
 
                char other[4];
                if (info_lnk.st_mode & S_IROTH) {
                    other[0] = 'R';
                } else {
                    other[0] = '-';
                }
 
                if (info_lnk.st_mode & S_IWOTH) {
                    other[1] = 'W';
                } else {
                    other[1] = '-';
                }
 
                if (info_lnk.st_mode & S_IXOTH) {
                    other[2] = 'X';
                } else {
                    other[2] = '-';
                }
                other[3] = '\0';
 
                snprintf(buffer, sizeof(buffer), "Nume legatura: %s\nDimensiune Legatura:%ld\nDrepturi de acces user legatura:%s\nDrepturi de acces grup legatura:%s\nDrepturi de acces altii legatura:%s\n", entry->d_name, info_lnk.st_size, owner, group, other);
            } else {
                return;
            }
        }
    }
 
    closedir(dir);
}
void aflare(const char *cale, const char *directorIesire,char c) {
    struct stat informatii;
    if (stat(cale, &informatii) == -1) {
        perror("Eroare la primirea informatii");
        exit(EXIT_FAILURE);
    }
 
    if (S_ISDIR(informatii.st_mode)) {
      director(cale, directorIesire,c);
    } else {
      fisier(cale, directorIesire,c);
    }
}
int main(int argc, char *argv[]) {
    if (argc != 4) {
        printf("Număr incorect de argumente\n");
        exit(EXIT_FAILURE);
    }
    char caracter = argv[3][0];
 
    aflare(argv[1], argv[2], caracter);
 
    return 0;
} 
