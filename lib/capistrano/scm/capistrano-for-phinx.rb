after "deploy:updating", "deploy:phinx_migrations"

namespace :deploy do
  task :restart do ; end

  desc 'Run Phinx migrations'
  task :phinx_migrations do
    on roles(:app) do
      within latest_release do
        execute 'vendor/bin/phinx', 'migrate', '-c', 'config/phinx.json', raise_on_non_zero_exit: false
      end
    end
  end
end
