site_dir      = Dir.pwd
git_tmp_dir   = `mktemp -d '/tmp/deploy-XXXXX'`.chomp!
build_tmp_dir = `mktemp -d '/tmp/deploy-XXXXX'`.chomp!

puts "Building CSS"
`sass css/site.sass css/site.css`

puts "Building site"
`jekyll #{site_dir} #{build_tmp_dir}`

Dir.chdir git_tmp_dir

puts "Changing to #{git_tmp_dir}"

puts "Cloning robb.github.com/monome"
`git clone git@github.com:robb/monome.js.git .`
`git checkout gh-pages`

puts "Syncing"
`rsync -f 'exclude .git' -r --delete #{build_tmp_dir}/ #{git_tmp_dir}`

puts "Committing changes"
`git add -A && git commit -m Deploying`

puts "Pushing..."
`git push`

