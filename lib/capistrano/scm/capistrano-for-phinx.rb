after "deploy:updating", "deploy:phinx_migrations"

namespace :deploy do
  task :restart do ; end

  desc 'Run Phinx migrations'
  task :phinx_migrations do
    on roles(:app) do
      within release_path do
        execute 'php', 'vendor/bin/phinx', 'migrate', '-c', 'config/phinx.json', raise_on_non_zero_exit: false
      end
    end
  end
end
