namespace :restarter do
  desc 'Restarts nginx'
  task :restart_nginx do
    on roles(:app) do
      execute 'echo sudo /bin/systemctl restart nginx'
    end
  end
  desc 'Restarts php-fpm'
  task :restart_php_fpm do
    on roles(:app) do
      execute 'echo sudo /bin/systemctl restart php-fpm'
    end
  end
end
