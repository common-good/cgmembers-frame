require 'capistrano/console'
set :application, "cgmembers-frame"
set :repo_url, 'git@github-cg:common-good/cgmembers-frame.git'
set :local_user, ENV['USER'] || ENV['USERNAME'] || `whoami`.chomp # shows (in the logs) who did the deployment

case (stage = fetch(:stage).to_s)   # whatever was typed after `cap`
when "test"
  ask :branch, `git rev-parse --abbrev-ref HEAD`.chomp # defaults to current checked-out branch
when "dev"
  set :branch, "develop"
when "staging"
  ask :branch, "main"
else # demo, beta, or main
  set :branch, "main"
end

server "#{stage}.commongood.earth", roles: %w{app db web}, user: stage, port: 7822
set :deploy_to, "/home/#{stage}/cg" # defaults to /var/www/my_app_name
set :pty, true # defaults to false
set :tmp_dir, "/home/#{stage}/tmp"

append :linked_files, "config/config.json"
append :linked_files, "config/phinx.json"
append :linked_dirs, "cgLogs", "cgPhotoTemp", "vendor", "cgmembers/.well-known"

set :keep_releases, 5 # defaults to 5

set :ssh_options, {
  # verify_host_key: :secure # Uncomment to require manually verifying the host key before first deploy.
  # keys: ENV['CG_DEPLOY_KEY'] ? [ENV['CG_DEPLOY_KEY']] : nil,  # nil if unset → Capistrano falls back to default discovery
  forward_agent: false,
  auth_methods: %w(publickey),
  user: stage,
  port: 7822  
}

# Admin-signin bootstrap files (§17) — test only. Requires shared/misc to be
# populated once (see §17's updated first step); this copies it into every
# fresh release automatically from then on. Doesn't replace §17's manual
# cleanup step (rm -rf .../misc, disable admin) after each redeploy — this
# only automates re-exposing it, not removing it again.
namespace :deploy do
  desc 'Copy admin bootstrap keys into place (test only)'
  task :copy_admin_keys do
    on roles(:app) do
      if fetch(:stage).to_s == "test"
        execute :cp, "-r", shared_path.join("misc").to_s, release_path.join("cgmembers/rcredits").to_s
      end
    end
  end
  after 'deploy:updated', 'deploy:copy_admin_keys'
end

after 'deploy:published', 'restarter:restart_nginx'
after 'deploy:published', 'restarter:restart_php_fpm'

# ask :branch, `git rev-parse --abbrev-ref HEAD`.chomp # defaults to develop (main branch is main)
# set :format, :airbrussh # defaults to :airbrussh
# set :default_env, { path: "/opt/ruby/bin:$PATH" } # defaults to {}
# set :local_user, -> { `git config user.name`.chomp } # defaults to ENV['USER']
# SSHKit.config.umask = '002' # Locals - this one sets the umask for the deployment

# You can configure the Airbrussh format using :format_options. These are the defaults:
# set :format_options, command_output: true, log_file: "log/capistrano.log", color: :auto, truncate: :auto
