package org.ppsspp.ppsspp;

import android.annotation.TargetApi;
import android.app.Activity;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.database.ContentObserver;
import android.net.Uri;
import android.os.Build;
import android.os.PowerManager;
import android.provider.Settings;

public class PowerSaveModeReceiver extends BroadcastReceiver {
	private static boolean isPowerSaving = false;
	private static boolean isBatteryLow = false;

	@Override
	public void onReceive(final Context context, final Intent intent) {
		final String action = intent.getAction();
		if (action.equals(Intent.ACTION_BATTERY_LOW)) {
			isBatteryLow = true;
		} else if (action.equals(Intent.ACTION_BATTERY_OKAY)) {
			isBatteryLow = false;
		}

		sendPowerSaving(context);
	}

	public static void initAndSend(final Activity activity) {
		sendPowerSaving(activity);

		activity.getContentResolver().registerContentObserver(Settings.System.CONTENT_URI, true, new ContentObserver(null) {
			@Override
			public void onChange(boolean selfChange, Uri uri) {
				super.onChange(selfChange, uri);

				String key = uri.getPath();
				key = key.substring(key.lastIndexOf("/") + 1, key.length());
				if (key != null && (key.equals("user_powersaver_enable") || key.equals("psm_switch"))) {
					PowerSaveModeReceiver.sendPowerSaving(activity);
				}
			}
		});
	}

	@TargetApi(21)
	private static boolean getNativePowerSaving(final Context context) {
		final PowerManager pm = (PowerManager)context.getSystemService(Context.POWER_SERVICE);
		return pm.isPowerSaveMode();
	}

	private static boolean getExtraPowerSaving(final Context context) {
		// http://stackoverflow.com/questions/25065635/checking-for-power-saver-mode-programically
		// HTC (Sense)
		String htcValue = Settings.System.getString(context.getContentResolver(), "user_powersaver_enable");
		if (htcValue != null && htcValue.equals("1")) {
			return true;
		}
		// Samsung (Touchwiz)
		String samsungValue = Settings.System.getString(context.getContentResolver(), "psm_switch");
		if (samsungValue != null && samsungValue.equals("1")) {
			return true;
		}
		return false;
	}

	private static void sendPowerSaving(final Context context) {
		if (Build.VERSION.SDK_INT >= 21) {
			isPowerSaving = getNativePowerSaving(context) || getExtraPowerSaving(context);
		} else {
			isPowerSaving = getExtraPowerSaving(context);
		}

		if (isBatteryLow || isPowerSaving) {
			NativeApp.sendMessage("core_powerSaving", "true");
		} else {
			NativeApp.sendMessage("core_powerSaving", "false");
		}
	}
}
