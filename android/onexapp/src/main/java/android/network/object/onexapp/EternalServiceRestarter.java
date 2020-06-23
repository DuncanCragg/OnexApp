// THANKS to https://fabcirablog.weebly.com/blog/creating-a-never-ending-background-service-in-android-gt-7

package network.object.onexapp;

import android.content.BroadcastReceiver;
import android.app.job.JobInfo;
import android.app.job.JobScheduler;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.util.Log;

import static android.content.Context.JOB_SCHEDULER_SERVICE;

public class EternalServiceRestarter extends BroadcastReceiver {
    private static String LOGNAME="Onex EternalServiceRestarter";

    private static JobScheduler jobScheduler;

    @Override
    public void onReceive(final Context context, Intent intent) {
        Log.d(LOGNAME, "*onReceive");
        if(jobScheduler==null) jobScheduler=(JobScheduler)context.getSystemService(JOB_SCHEDULER_SERVICE);
        ComponentName componentName = new ComponentName(context, EternalServiceJob.class);
        JobInfo jobInfo = new JobInfo.Builder(1, componentName).setOverrideDeadline(0).setPersisted(true).build();
        jobScheduler.schedule(jobInfo);
    }
}
