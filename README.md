Common Good&reg; Democratic Economics System
Copyright (c) 2022 Society to Benefit Everyone, Inc., dba Common Good
=====================================================================

This community-centered payment system is designed to provide greater local control of economics, so we can make our communities everything we want them to be.

The companion software project, Common Good POS, works very much like credit card processing apps for swipe cards. Except instead of a swipe card reader you use your smartphone's camera vision to scan a QR code on the customer's Common Good Card.

Set up instructions are located on Google Drive [here](https://docs.google.com/document/d/1oQU-PPown2TV02Xg9htxByzOhUopGH-areUrlZ94Te0/edit).

## Docker setup for development

There is tooling to build and orchestrate the dependencies needed to run this software using [docker](https://www.docker.com/) and [docker-compose](https://docs.docker.com/compose/install/).

From the project directory

* Build and start the containers: `docker-compose up`
* Install dependencies and run migrations: `docker-compose exec app /var/www/init.sh`
* To get a shell to the running container: `docker-compose exec -it app bash`