from socket import timeout
import pathlib
import os
from time import sleep
import subprocess

Import("env")

path =  os.path.dirname(pathlib.Path().parent.absolute())
exe = path + '/SerialUtils.exe'
  
def before_upload(source, target, env):
  print('NuvIoT - Setting Boot Loader')
  process = subprocess.run([exe, 'COM42', 'b'])
  sleep(1)
  print('NuvIoT - Set Boot Loader')
  print('returncode', process.returncode)  

def after_upload(source, target, env):
  print('NuvIoT - Restting ESP')
  process = subprocess.run([exe, 'COM42', 'r'])
  print('NuvIoT - Reset Chip')
  print('returncode', process.returncode)


env.AddPreAction("upload", before_upload)
env.AddPostAction("upload", after_upload)