cd ~
sudo apt-get update
sudo apt-get install unzip
wget https://dl.google.com/android/repository/sdk-tools-linux-4333796.zip
unzip sdk-tools-linux-4333796.zip -d Android
rm sdk-tools-linux-4333796.zip
sudo apt-get install -y lib32z1 openjdk-8-jdk
export JAVA_HOME=/usr/lib/jvm/java-8-openjdk-amd64
export PATH=$PATH:$JAVA_HOME/bin
printf "\n\nexport JAVA_HOME=/usr/lib/jvm/java-8-openjdk-amd64\nexport PATH=\$PATH:\$JAVA_HOME/bin" >> ~/.bashrc
cd Android/tools/bin
./sdkmanager "platform-tools" "platforms;android-26" "build-tools;26.0.3"
export ANDROID_HOME=$HOME/Android
export PATH=$ANDROID_HOME/tools:$PATH
export PATH=$ANDROID_HOME/tools/bin:$PATH
export PATH=$ANDROID_HOME/platform-tools:$PATH
printf "\n\nexport ANDROID_HOME=$HOME/Android\nexport PATH=\$PATH:\$ANDROID_HOME/tools/bin\nexport PATH=\$PATH:\$ANDROID_HOME/platform-tools" >> ~/.bashrc
sdkmanager --update
sudo apt-get install gradle
gradle -v
sdkmanager "ndk-bundle"
# adb start-server