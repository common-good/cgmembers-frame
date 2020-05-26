DROP DATABASE cg;
DROP USER IF EXISTS cg@localhost;
CREATE DATABASE cg;
CREATE USER cg@localhost IDENTIFIED BY 'cg';
GRANT ALL ON cg.* TO cg@localhost;
USE cg;
SOURCE db/startup.sql;
set global log_bin_trust_function_creators = 1;
