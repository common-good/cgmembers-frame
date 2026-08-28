namespace :restarter do
  desc 'Reloads nginx'
  task :restart_nginx do
    on roles(:app) do
      execute 'sudo /bin/systemctl reload nginx'
    end
  end

  desc 'Reloads php-fpm'
  task :restart_php_fpm do
    on roles(:app) do
      execute 'sudo /bin/systemctl reload php8.2-fpm'
    end
  end
end
