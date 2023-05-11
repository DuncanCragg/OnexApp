// THANKS to https://fabcirablog.weebly.com/blog/creating-a-never-ending-background-service-in-android-gt-7

package network.object.onexapp;

import java.util.List;

import android.app.job.JobParameters;
import android.content.*;
import android.content.pm.*;
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

        Intent intent = new Intent("network.object.onexapp.eternal.restart").setFlags(Intent.FLAG_INCLUDE_STOPPED_PACKAGES);
        PackageManager packageManager = getPackageManager();
        List<ResolveInfo> broadcastReceivers = packageManager.queryBroadcastReceivers(intent, 0);
        for(ResolveInfo broadcastReceiver: broadcastReceivers) {
            ComponentName cn = new ComponentName(broadcastReceiver.activityInfo.packageName, broadcastReceiver.activityInfo.name);
            intent.setComponent(cn);
            sendBroadcast(intent);
        }
        return false;
    }
}
