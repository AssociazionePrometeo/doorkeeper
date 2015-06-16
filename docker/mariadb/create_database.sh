#!/bin/bash

# Wait a few seconds
declare -i t
t=10
echo "waiting $t seconds..." 
sleep $t
echo "create database using script: /tmp/create_database.sql"

# create database
/usr/bin/mysql -u root -p$MYSQL_ROOT_PASSWORD < /tmp/create_database.sql
