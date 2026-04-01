//
// Created by jude on 11/02/2026.
//

#ifndef UNTITLED3_DB_H
#define UNTITLED3_DB_H
#include <sqlite3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>



int add_user(sqlite3 *db, unsigned char *public_key, char *user_name, int user_name_len) {
    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(db, "insert into Users(uid, public_key, username) values(NULL, ?, ?)", -1, &stmt,NULL);
    sqlite3_bind_blob(stmt, 1, public_key, 8,SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, user_name, user_name_len,SQLITE_STATIC);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    return sqlite3_last_insert_rowid(db);
}


unsigned char* get_public_key(sqlite3 *db, uint32_t uid) {
    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(db, "SELECT public_key FROM Users WHERE uid = ?", -1, &stmt,NULL);
    sqlite3_bind_int(stmt, 1, uid);
    sqlite3_step(stmt);
    unsigned char *public_key = malloc(crypto_sign_PUBLICKEYBYTES);
    memcpy(public_key, sqlite3_column_blob(stmt, 0), crypto_sign_PUBLICKEYBYTES);
    sqlite3_finalize(stmt);
    return public_key;
}

#endif //UNTITLED3_DB_H
