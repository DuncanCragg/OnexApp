// THANKS to https://fabcirablog.weebly.com/blog/creating-a-never-ending-background-service-in-android-gt-7

package network.object.onexapp;

import android.app.job.JobParameters;
import android.content.*;
import android.util.Log;

public class EternalServiceJob extends android.app.job.JobService {
    private static String LOGNAME="Onex EternalServiceJob";

    @Override
    public boolean onStartJob(JobParameters jobParameters) {
        Log.d(LOGNAME, "*onStartJob");

        startForegroundService(new Intent(this, EternalService.class));
        return false;
    }

    @Override
    public boolean onStopJob(JobParameters jobParameters) {
        Log.d(LOGNAME, "*onStopJob");

        OnexNativeActivity.restartEternal();

        return false;
    }
}
