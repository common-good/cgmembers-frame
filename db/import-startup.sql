DROP DATABASE IF EXISTS cg;
CREATE DATABASE cg;

DROP USER IF EXISTS cg_user@localhost;
FLUSH PRIVILEGES;

CREATE USER cg_user@localhost IDENTIFIED BY 'S7Gh$45%h1t!#';
GRANT ALL PRIVILEGES ON cg.* TO cg_user@localhost;
FLUSH PRIVILEGES;

USE cg;
SOURCE db/startup.sql;
set global log_bin_trust_function_creators = 1;
