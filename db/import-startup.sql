DROP DATABASE IF EXISTS test;
CREATE DATABASE test;

DROP USER IF EXISTS test@localhost;
FLUSH PRIVILEGES;

CREATE USER test@localhost IDENTIFIED BY 'pass';
GRANT ALL PRIVILEGES ON test.* TO test@localhost;
FLUSH PRIVILEGES;

USE test;
SOURCE db/startup.sql;
set global log_bin_trust_function_creators = 1;
