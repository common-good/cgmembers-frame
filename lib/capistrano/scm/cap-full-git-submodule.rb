require "capistrano/plugin"

class Capistrano::SCM::Git::FullSubmodules < Capistrano::Plugin

  def set_defaults
    set_if_empty :submodule_repos_path, 'sub-repos'
    set_if_empty :git_shallow_clone, false
    set_if_empty :git_submodule_shallow_clone, fetch(:git_shallow_clone)
  end
  
  def register_hooks
    after 'git:check', 'git:full_submodules:check'
    after 'git:clone', 'git:full_submodules:clone'
    after 'git:update', 'git:full_submodules:update'
    after 'git:create_release', 'git:full_submodules:create_release_for_submodules'
  end

  # returns a list of submodule names
  def submodule_names
    x = ''
    x = %x(git config --blob HEAD:.gitmodules --list)
    # on roles(:app).first do
    #   within repo_path do
    #     x = capture(:git, :config, '--blob', 'HEAD:.gitmodules', '--list')
    #   end
    # end
    names = x.split("\n").map { |entry|
      md = %r{^submodule\.([-_a-zA-Z0-9/]*)\.(path|url)=(.*)$}.match(entry)
      if md[2] == 'path' then md[1] else nil end
    }.reject &:nil?
  end
  
  def submodule_mirror_exists?(name)
    backend.test " [ -d #{submodule_mirror_path(name).to_s} ] "
  end

  def submodule_repo_url(name)
    url = ''
    # on :local do
      # within name do
        url = %x|(cd #{name.to_s} && git remote get-url origin)|
      # end
    # end
    url.strip
  end

  def check_submodule_repo_is_reachable(name)
    git :'ls-remote', submodule_repo_url(name), "HEAD"
  end

  def clone_submodule(name)
    depth = fetch(:git_submodule_shallow_clone)
    puts depth.to_s + ', '
    if (depth)
      git :clone, "--mirror", "--depth", depth.to_s, "--no-single-branch", submodule_repo_url(name), submodule_mirror_path(name).to_s
    else
      git :clone, "--mirror", submodule_repo_url(name), submodule_mirror_path(name).to_s
    end
  end

  def submodule_mirror_path(name)
    File.join(fetch(:deploy_to), fetch(:submodule_repos_path), name)
  end
  
  def update_submodule_mirror(name)
    # # Update the origin URL if necessary.
    # git :remote, "set-url", "origin", submodule_repo_url(name)

    # Note: Requires git version 1.9 or greater
    depth = fetch(:git_submodule_shallow_clone)
    if (depth)
      git :fetch, "--depth", depth.to_s, "origin", branch # fetch(:branch)
    else
      git :fetch, 'origin'
      # git :remote, :update, "--prune"
    end
  end

  def submodule_branch(name)
    x = ''
    on roles(:app).first do
      within repo_path do
        x = capture(:git, 'ls-tree', fetch(:branch), name)
      end
    end
    x.split[2]
  end
    
  def submodule_archive_to_release_path(name)
    # if (tree = fetch(:repo_tree))
    #   tree = tree.slice %r#^/?(.*?)/?$#, 1
    #   components = tree.split("/").size
    #   git :archive, fetch(:branch), tree, "| #{SSHKit.config.command_map[:tar]} -x --strip-components #{components} -f - -C", release_path
    # else
      git :archive, submodule_branch(name), "| #{SSHKit.config.command_map[:tar]} -x -f - -C", submodule_release_path(name)
    # end
  end

  def submodule_release_path(name)
    File.join(release_path, name)
  end
  
  def git(*args)
    args.unshift :git
    backend.execute(*args)
  end

  # def git_sub_repo_url(sub_repo_url)
  #   if fetch(:git_http_username) && fetch(:git_http_password)
  #     URI.parse(sub_repo_url).tap do |repo_uri|
  #       repo_uri.user     = fetch(:git_http_username)
  #       repo_uri.password = CGI.escape(fetch(:git_http_password))
  #     end.to_s
  #   elsif fetch(:git_http_username)
  #     URI.parse(sub_repo_url).tap do |repo_uri|
  #       repo_uri.user = fetch(:git_http_username)
  #     end.to_s
  #   else
  #     sub_repo_url
  #   end
  # end
  

  def define_tasks
    git_plugin = self  # This trick lets us access the Git plugin within `on` blocks.

    namespace :git do
      namespace :full_submodules do
        
        desc "Get list of submodules"
        task get_list: :'git:wrapper' do
          puts("Getting list")
          for x in git_plugin.submodule_names do
            puts x + ' ' + submodule_branch(x)
          end
        end
        
        desc "Check that all submodule repositories are reachable"
        task check: :'git:wrapper' do
          for name in git_plugin.submodule_names do
            on release_roles :all do
              git_plugin.check_submodule_repo_is_reachable name
            end
          end
        end

        desc "Clone the submodule-repos to their mirrors"
        task clone: :'git:wrapper' do
          for name in submodule_names do
            on release_roles :all do
              if git_plugin.submodule_mirror_exists?(name)
                info t(:mirror_exists, at: git_plugin.submodule_mirror_path(name))
              else
                within deploy_path do
                  with fetch(:git_environmental_variables) do
                    git_plugin.clone_submodule name
                  end
                end
              end
            end
          end
        end

        desc "Update the submodule mirrors to reflect the origin state"
        task update: :'git:full_submodules:clone' do
          for name in submodule_names do
            on release_roles :all do
              within git_plugin.submodule_mirror_path(name) do
                with fetch(:git_environmental_variables) do
                  git_plugin.update_submodule_mirror(name)
                end
              end
            end
          end
        end
        
        desc "Copy submodule repo to releases"
        task create_release_for_submodules: :'git:full_submodules:update' do
          for name in submodule_names do
            on release_roles :all do
              with fetch(:git_environmental_variables) do
                execute :mkdir, "-p", git_plugin.submodule_release_path(name).to_s
                within git_plugin.submodule_mirror_path(name) do
                  git_plugin.submodule_archive_to_release_path(name)
                end
              end
            end
          end
        end

#   desc "Determine the revision that will be deployed"
#   task :set_current_revision do
#     on release_roles :all do
#       within repo_path do
#         with fetch(:git_environmental_variables) do
#           set :current_revision, git_plugin.fetch_revision
#         end
#       end
      end
    end
  end
end
