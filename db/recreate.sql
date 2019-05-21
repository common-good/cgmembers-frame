drop database cg;
create database cg;
grant all on cg.* to cg@localhost;
use cg;
source db/startup.sql;
