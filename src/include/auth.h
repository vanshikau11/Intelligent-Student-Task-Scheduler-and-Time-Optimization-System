#ifndef AUTH_H_
#define AUTH_H_

#include "main.h"

int checkCredentials(const char *id, const char *password);
int idExists(const char *id);
void addUser(const char *name, const char *id, const char *password);
int isValidName(const char *name);
int isValidPassword(const char *password);

#endif // AUTH_H_