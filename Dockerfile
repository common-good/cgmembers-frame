FROM php:8.2-apache

RUN apt-get update -y && apt install -y \
  libbz2-dev \
  curl \
  libcurl4-openssl-dev \
  openssl \
  libfreetype-dev \
  libjpeg62-turbo-dev \
  libpng-dev \
  gettext \
  libonig-dev \
  libsqlite3-dev \
  mariadb-client \
  wget \
  git \
  unzip \
  libicu-dev

RUN docker-php-ext-configure gd --with-jpeg \
	&& docker-php-ext-install -j$(nproc) bz2 curl fileinfo gd gettext mbstring exif mysqli pdo_mysql pdo_sqlite intl

RUN a2enmod rewrite

# install init scripts and composer
WORKDIR /opt/util

RUN wget https://raw.githubusercontent.com/composer/getcomposer.org/main/web/installer -O - -q | php -- --quiet
RUN mv composer.phar /usr/local/bin/composer

WORKDIR /var/www

ADD docker/php.ini.development /usr/local/etc/php/php.ini
ADD composer.json .
ADD db/ db/

ADD docker/init.sh .
RUN chmod 755 init.sh

# application files will be added via volume mount in compose.
