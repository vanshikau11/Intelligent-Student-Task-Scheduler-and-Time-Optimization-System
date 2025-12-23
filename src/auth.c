#include "main.h"
#include <ctype.h>  
#include <string.h>

/* ---------------- Authentication ---------------- */
int checkCredentials(const char *id, const char *password) {
    FILE *fp = fopen("data/users.txt","r");
    if (!fp) return 0;
    char name[64], fileId[64], filePass[64];
    while (fscanf(fp,"%63s %63s %63s", name, fileId, filePass) != EOF) {
        if (strcmp(fileId, id) == 0 && strcmp(filePass, password) == 0) { fclose(fp); return 1; }
    }
    fclose(fp); return 0;
}
//check id
int idExists(const char *id) {
    FILE *fp = fopen("data/users.txt","r");
    if (!fp) return 0;
    char name[64], fileId[64], filePass[64];
    while (fscanf(fp,"%63s %63s %63s", name, fileId, filePass) != EOF) if (strcmp(fileId, id) == 0) { fclose(fp); return 1; }
    fclose(fp); return 0;
}
//add new user
void addUser(const char *name, const char *id, const char *password) {
    FILE *fp = fopen("data/users.txt","a");
    if (!fp) return;
    fprintf(fp, "%s %s %s\n", name?name:"NoName", id?id:"nouser", password?password:"");
    fclose(fp);
}
int isValidName(const char *name) {
    if (strlen(name) == 0) return 0; // Cannot be empty

    for (int i = 0; name[i] != '\0'; i++) {
        // if the character is NOT an alphabet letter AND NOT a space
        if (!isalpha(name[i]) && !isspace(name[i])) {
            return 0; // Found an invalid character
        }
    }
    return 1; // All characters are valid
}

/*
 * Checks if the password meets the complexity requirements.
 */
int isValidPassword(const char *password) {
    int hasUpper = 0, hasLower = 0, hasDigit = 0, hasSpecial = 0;
    int len = 0;

    for (len = 0; password[len] != '\0'; len++) {
        char c = password[len];
        if (isupper(c)) hasUpper = 1;
        if (islower(c)) hasLower = 1;
        if (isdigit(c)) hasDigit = 1;
        if (ispunct(c)) hasSpecial = 1; // ispunct() checks for special chars
    }

    if (len < 8) return 0; // Check minimum length

    // Return 1 (true) only if all conditions are met
    return hasUpper && hasLower && hasDigit && hasSpecial;
}