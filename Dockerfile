FROM centos:7

WORKDIR /code
RUN yum update -y
RUN yum install -y epel-release
RUN yum install -y nginx php
# ruby git vim php
# COPY Gemfile Gemfile.lock ./
# RUN bundle install
CMD nginx -g 'daemon off;'
# CMD tail -f /dev/null
