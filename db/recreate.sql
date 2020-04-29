DROP DATABASE cg;
DROP USER IF EXISTS cg@localhost;
CREATE DATABASE cg;
CREATE USER cg@localhost IDENTIFIED BY '';
GRANT ALL ON cg.* TO cg@localhost;
USE cg;
SOURCE db/startup.sql;
