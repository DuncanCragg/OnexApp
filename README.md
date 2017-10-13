
# Onex

### Quickstart:

```
$ git clone --recursive git@github.com:DuncanCragg/OnexApp.git
$ cd OnexApp/
$ cmake .
$ make
$ cd bin; ./OnexApp

$ cd android
$ (copy your local.properties over)
$ ./gradlew build --parallel
$ adb -d uninstall network.object.onexapp && adb -d install ./onexapp/build/outputs/apk/onexapp-debug.apk

$ adb logcat OnexApp:D \*:S
$ adb logcat | grep -F "`adb shell ps | grep network.object.onexapp | cut -c10-15`"
```


Read about the [initial plans for Onex here](http://object.network/onex-app.html).


