# commands to deploy a Common Good development machine using Ansible playbooks


# before running this script, please make sure all files and values are correctly 
# set in the files found in:
#  templates/
#  vars/
# and the file:
#  commongood-dev.inventory


# this script presumes all install and setup steps completed previously

# deploy (only) common good software
ansible-playbook -i commongood-dev.inventory commongood-30-deploy.yaml

# apply hacks to config, scripts, etc.
ansible-playbook -i commongood-dev.inventory commongood-99-hacks.yaml

