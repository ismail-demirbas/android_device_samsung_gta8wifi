
package org.lineageos.folio;

import android.os.SystemProperties;
import android.util.Log;

import com.android.settingslib.drawer.EntriesProvider;
import com.android.settingslib.drawer.EntryController;
import com.android.settingslib.drawer.ProviderSwitch;

import java.util.Collections;
import java.util.List;

public final class FolioDaemonProvider extends EntriesProvider {

    private static final String TAG = "FolioDaemonSettings";
    private static final String PROP = "persist.sys.folio.enabled";

    @Override
    protected List<? extends EntryController> createEntryControllers() {
        return Collections.singletonList(new FolioDaemonController());
    }

    static final class FolioDaemonController extends EntryController
            implements ProviderSwitch {

        @Override
        public String getKey() {
            return "folio_daemon";
        }

        @Override
        protected MetaData getMetaData() {
            return new MetaData("com.android.settings.category.ia.display")
                    .setTitle("Folio Daemon")
                    .setSummary("Automatically wakes or sleeps the screen when opening or closing the magnetic folio cover.");
        }

        @Override
        public boolean isSwitchChecked() {
            return SystemProperties.getBoolean(PROP, false);
        }

        @Override
        public boolean onSwitchCheckedChanged(boolean checked) {
            try {
                SystemProperties.set(PROP, checked ? "1" : "0");
                android.provider.Settings.Global.putInt(android.app.ActivityThread.currentApplication().getContentResolver(), "lid_behavior", checked ? 1 : 0);

                return SystemProperties.getBoolean(PROP, false) == checked;
            } catch (RuntimeException e) {
                Log.e(TAG, "Unable to change folio state", e);
                return false;
            }
        }

        @Override
        public String getSwitchErrorMessage(boolean attemptedChecked) {
            return "Unable to change Folio Daemon state";
        }
    }
}
