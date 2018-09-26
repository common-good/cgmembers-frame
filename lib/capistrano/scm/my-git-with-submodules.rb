require "capistrano/scm/git"

class Capistrano::SCM::Git::MyWithSubmodules < Capistrano::SCM::Git

  def register_hooks
    after 'git:create_release', 'git:mysubmodules:create_release'
  end

  def define_tasks
    namespace :git do
      namespace :mysubmodules do

        desc "Adds configured submodules recursively to release"
        task create_release: :'git:update' do
          temp_work_path = release_path.join("TEMP_WORK_PATH#{fetch(:release_timestamp)}")
          on release_roles :all do
            with fetch(:git_environmental_variables) do
              # within repo_path do
              execute :git, :clone, '--recursive', repo_path.to_s, temp_work_path.to_s
              quiet = Rake.application.options.trace ? '' : '--quiet'
              # execute :git, :submodule, 'sync', '--recursive', quiet
                
              within temp_work_path do
                execute :git, :submodule, :sync, '--recursive', quiet
                execute :git, :submodule, :update, '--init', '--checkout', '--recursive', quiet
                if (tree = fetch(:repo_tree))
                  tree = tree.slice %r#^/?(.*?)/?$#, 1
                  components = tree.split("/").size
                  execute :git, :submodule, :foreach, '--recursive', "'echo $displaypath && cd $displaypath && git archive --prefix=$displaypath #{fetch(:branch)} #{tree} | #{SSHKit.config.command_map[:tar]} -x --strip-components #{components} -f - -C #{release_path}'"
                else
                  execute :git, :submodule, :foreach, '--recursive', "'echo $toplevel/$sm_path && cd $toplevel/$sm_path && git archive --prefix=$displaypath #{fetch(:branch)} | #{SSHKit.config.command_map[:tar]} -x -f - -C #{release_path}'"
                end
              end
              # execute :rm, '-rf', temp_work_path.to_s
            end
          end
        end
      end
    end
  end
end
