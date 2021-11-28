# commands to deploy a Common Good development machine using Ansible playbooks


# before running this script, please make sure all files and values are correctly 
# set in the files found in:
#  templates/
#  vars/
# and the file:
#  commongood-dev.inventory


# first install Ansible role prerequisites
ansible-galaxy install --role-file requirements.yaml

# create and provision a Vagrant (on VirtualBox)
vagrant up

# deploy the Common Good software to the newly provisioned machine

# if running standalone on an existing box/VM (i.e. not 
# already provisioned by vagrant up) execute this line:
#ansible-playbook -i commongood-dev.inventory commongood-01-provision.yaml

# create users, etc.
ansible-playbook -i commongood-dev.inventory commongood-05-base.yaml

# install software prerequisites
ansible-playbook -i commongood-dev.inventory commongood-10-prereqs.yaml

# install common good software
ansible-playbook -i commongood-dev.inventory commongood-20-install.yaml

# commission and build common good database
ansible-playbook -i commongood-dev.inventory commongood-25-install-database.yaml

# deploy (only) common good software
ansible-playbook -i commongood-dev.inventory commongood-30-deploy.yaml

# apply hacks to config, scripts, etc.
ansible-playbook -i commongood-dev.inventory commongood-99-hacks.yaml

