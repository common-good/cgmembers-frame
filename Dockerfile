FROM nginx:latest

WORKDIR /code
RUN apt update
RUN apt install -y ruby bundler git vim php
COPY Gemfile Gemfile.lock ./
RUN bundle install
CMD nginx
