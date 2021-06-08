FROM nginx

WORKDIR /code
RUN apt update -y
# RUN apt install -y epel-release
RUN apt install -y php
# ruby git vim php
# COPY Gemfile Gemfile.lock ./
# RUN bundle install
# CMD nginx -g 'daemon off;'
# CMD tail -f /dev/null
