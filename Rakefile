# -*- ruby -*-

require 'rubygems'
require 'hoe'

Hoe.spec 'fft' do
  developer('Jude Sutton', 'jude.sutton@gmail.com')
  spec_extras[:extensions] = "ext/extconf.rb" 
  extra_dev_deps << ['rake-compiler', '>= 0']
  
  Rake::ExtensionTask.new('fft', spec) do |ext|
    ext.lib_dir = File.join('lib', 'fft')
  end
  # self.rubyforge_name = 'fftx' # if different than 'fft'
end

# vim: syntax=ruby

