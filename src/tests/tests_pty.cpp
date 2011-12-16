//////////////////////////////////////////////////////////////////
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/select.h>
#include <errno.h>
#include <sys/wait.h>
#include <string>
#include <string.h>
#include <cstdio>
#include <pty.h>
//#include <utmp.h>

//static int   read_handle(-1);
//static pid_t pid;
//
//bool read_from_child(std::string& buff) {
//    fd_set  rs;
//    timeval timeout;
//
//    memset(&rs, 0, sizeof(rs));
//    FD_SET(read_handle, &rs);
//    timeout.tv_sec  = 1; // 1 second
//    timeout.tv_usec = 0;
//
//    int rc = select(read_handle+1, &rs, NULL, NULL, &timeout);
//    if ( rc == 0 ) {
//        // timeout
//        return true;
//
//    } else if ( rc > 0 ) {
//        // there is something to read
//        char buffer[1024*64]; // our read buffer
//        memset(buffer, 0, sizeof(buffer));
//        if(read(read_handle, buffer, sizeof(buffer)) > 0) {
//            buff.clear();
//            buff.append( buffer );
//            return true;
//        }
//
//        return false;
//    } else { /* == 0 */
//        if ( rc == EINTR || rc == EAGAIN ) {
//            return true;
//        }
//
//        // Process terminated
//        int status(0);
//        waitpid(pid, &status, 0);
//        return false;
//    }
//}
//
//void execute() {
//    char *argv[] = {(const char*)"ls", NULL};
//    int    argc = 1;
//
//    int master, slave;
//    openpty(&master, &slave, NULL, NULL, NULL);
//
//    int rc = fork();
//    if ( rc == 0 ) {
//        login_tty(slave);
//        close(master);
//
//        // execute the process
//        if(execvp(argv[0], argv) != 0)
//            perror("execvp");
//
//    } else if ( rc < 0 ) {
//        perror("fork");
//        return;
//
//    } else {
//        // Parent
//        std::string buf;
//        close(slave);
//
//        read_handle = master;
//        while(read_from_child(buf)) {
//            if(buf.empty() == false) {
//                printf("Received: %s", buf.c_str());
//            }
//            buf.clear();
//        }
//    }
//}
//
//int non_posix_pty() {
//    execute();
//    return 0;
//}




////////////////////////////////////////////////////////////////:
// Posix pty
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <termios.h>
#include <iostream>
#include <string>

int posix_pty() {
	int fdm, fds, rc;
		char input[150];
		fdm = posix_openpt(O_RDWR);
		if (fdm < 0) {
			fprintf(stderr, "Erreur %d sur posix_openpt()\n", errno);
			return 1;
		}
		rc = grantpt(fdm);
		if (rc != 0) {
			fprintf(stderr, "Erreur %d sur grantpt()\n", errno);
			return 1;
		}
		rc = unlockpt(fdm);
		if (rc != 0) {
			fprintf(stderr, "Erreur %d sur unlockpt()\n", errno);
			return 1;
		}
		// Ouverture du PTY esclave
		fds = open(ptsname(fdm), O_RDWR);
		// Création d'un processus fils
		if (fork()) {
			// Code du processus pere
			// Fermeture de la partie esclave du PTY
			close(fds);
			while (1) {
				// Saisie operateur (entree standard = terminal)
				write(1, "Entree : ", sizeof("Entree : "));
				rc = read(0, input, sizeof(input));
				if (rc > 0) {
					// Envoie de la saisie aux processus fils via le PTY
					write(fdm, input, rc);
					// Lecture de la reponse du fils dans le PTY
					rc = read(fdm, input, sizeof(input) - 1);
					if (rc > 0) {
						// Ajout d'une fin de chaine en fin de buffer
						input[rc] = '\0';

						fprintf(stderr, "%s", input);
					} else {
						break;
					}
				} else {
					break;
				}
			} // End while
		} else {
	//		struct termios slave_orig_term_settings; // Saved terminal settings
	//		struct termios new_term_settings; // Current terminal settings
			// Code du processus fils
			// Fermeture de la partie maitre du PTY
			close(fdm);
			// Sauvegarde des parametre par defaut du PTY esclave
	//		rc = tcgetattr(fds, &slave_orig_term_settings);
			// Positionnement du PTY esclave en mode RAW
	//		new_term_settings = slave_orig_term_settings;
	//		cfmakeraw(&new_term_settings);
	//		tcsetattr(fds, TCSANOW, &new_term_settings);
			// Le cote esclave du PTY devient l'entree et les sorties standards du fils
			close(0); // Fermeture de l'entrée standard (terminal courant)
			close(1); // Fermeture de la sortie standard (terminal courant)
			close(2); // Fermeture de la sortie erreur standard (terminal courant)
			dup(fds); // Le PTY devient l'entree standard (0)
			dup(fds); // Le PTY devient la sortie standard (1)
			dup(fds); // Le PTY devient la sortie erreur standard (2)
			while (1) {
				rc = read(fds, input, sizeof(input) - 1);
				if (rc > 0) {
					// Remplacement du retour a la ligne par une fin de chaine
					input[rc - 1] = '\0';
					printf("Le fils a recu : '%s'\n", input);
				} else {
					break;
				}
			} // End while
		}
		return 0;
}


// return true if something to return
bool getLine(int fd, std::string &str) {
	char buffer;
	int rc;
	str = "";

	while (read(fd, &buffer, sizeof(buffer)) > 0) {
		// on a pas fini le fichier
		str += buffer;
		if (buffer == '\n') {
			break;
		}
	}
	return str.size() != 0;
}

int my_posix_pty() {
	int master, slave, rc;
	char input[150];
	master = posix_openpt(O_RDWR);
	if (master < 0) {
		fprintf(stderr, "Erreur %d sur posix_openpt()\n", errno);
		return 1;
	}
	rc = grantpt(master);
	if (rc != 0) {
		fprintf(stderr, "Erreur %d sur grantpt()\n", errno);
		return 1;
	}
	rc = unlockpt(master);
	if (rc != 0) {
		fprintf(stderr, "Erreur %d sur unlockpt()\n", errno);
		return 1;
	}
	// Ouverture du PTY esclave
	slave = open(ptsname(master), O_RDWR);
	// Création d'un processus fils
	if (fork()) {
		// Code du processus pere
		// Fermeture de la partie esclave du PTY
		close(slave);
		std::string str;
		while (getLine(master, str)) {
			std::cout << str;
		}
		std::cout << "end while" << std::endl;
	} else {
		struct termios slave_orig_term_settings; // Saved terminal settings
		struct termios new_term_settings; // Current terminal settings
		// Code du processus fils
		// Fermeture de la partie maitre du PTY
		close(master);
		// Sauvegarde des parametre par defaut du PTY esclave
//		rc = tcgetattr(slave, &slave_orig_term_settings);
//		new_term_settings = slave_orig_term_settings; // Positionnement du PTY esclave en mode RAW
//		cfmakeraw(&new_term_settings);
//		tcsetattr(slave, TCSANOW, &new_term_settings);

		// Le cote esclave du PTY devient l'entree et les sorties standards du fils
		close(STDIN_FILENO); // Fermeture de l'entrée standard (terminal courant)
		close(STDOUT_FILENO); // Fermeture de la sortie standard (terminal courant)
		close(STDERR_FILENO); // Fermeture de la sortie erreur standard (terminal courant)

		dup2(slave, STDIN_FILENO); // pt_esclave devient stdin
		dup2(slave, STDOUT_FILENO); // pt_esclave devient stdout
		dup2(slave, STDERR_FILENO); // pt_esclave devient stderr
		char* argv[] = { "yes", 0 };
		execvp("yes", argv);
	} // End while
	return 0;
}


int my_posix_pty2() {
	int master;
	char input[150];

	// Création d'un processus fils
	if (forkpty(&master, NULL, NULL, NULL)) {
		// Code du processus pere
		// Fermeture de la partie esclave du PTY
		std::string str;
		while (getLine(master, str)) {
			std::cout << str;
		}
		std::cout << "end while" << std::endl;
	} else {
		// Fermeture de la partie maitre du PTY
		close(master);
		char* argv[] = { "yes", 0 };
		execvp("yes", argv);
	} // End while
	return 0;
}




int main(void) {
	return my_posix_pty2();
//	return non_posix_pty();
	//	return posix_pty();
}
