FROM php:7.3-fpm

WORKDIR /code
RUN docker-php-ext-install pdo_mysql
RUN apt-get update -y
RUN apt-get install -y default-mysql-client
# RUN apt-get install -y ruby git vim php
# COPY Gemfile Gemfile.lock ./
# RUN bundle install
# RUN ssh db -L 3306:localhost:3306
# COPY db/import-startup.sql db/startup.sql ./db/
# RUN mysql -h db < ./db/import-startup.sql
# RUN vendor/robmorgan/phinx/bin/phinx migrate -e development -c config/phinx.json
