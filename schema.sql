CREATE TABLE Users(
    uid INTEGER PRIMARY KEY,
    public_key BLOB NOT NULL,
    username TEXT UNIQUE
);