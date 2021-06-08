FROM centos:7

WORKDIR /code
RUN yum update -y
RUN yum install -y nginx ruby git vim php
COPY Gemfile Gemfile.lock ./
# RUN bundle install
CMD nginx
