
package network.object.onexapp;

import android.content.*;
import android.app.*;

public class AlarmReceiver extends BroadcastReceiver {

    @Override
    public void onReceive(Context context, Intent intent) {
        showNotification(context, "Onex", "Alarm");
    }

    public void showNotification(Context context, String title, String text){
         PendingIntent pendingIntent = PendingIntent.getActivity(context, 0, new Intent(context, OnexNativeActivity.class), PendingIntent.FLAG_UPDATE_CURRENT);
         Notification.Builder notifbuilder = new Notification.Builder(context);
         notifbuilder.setContentTitle(title)
                     .setContentText(text)
                     .setSmallIcon(R.drawable.icon)
                     .setContentIntent(pendingIntent);
         NotificationManager notifMgr = (NotificationManager)context.getSystemService(context.NOTIFICATION_SERVICE);
         notifMgr.notify(12345, notifbuilder.build());
    }
}
