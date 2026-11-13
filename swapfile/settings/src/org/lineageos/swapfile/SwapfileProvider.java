package org.lineageos.swapfile;

import android.os.SystemProperties;

import com.android.settingslib.drawer.EntriesProvider;
import com.android.settingslib.drawer.EntryController;
import com.android.settingslib.drawer.ProviderSwitch;

import java.util.Collections;
import java.util.List;

public class SwapfileProvider extends EntriesProvider {
    static final String PROP = "persist.sys.swapfile.enabled";

    @Override
    protected List<? extends EntryController> createEntryControllers() {
        return Collections.singletonList(new SwapfileController());
    }

    static class SwapfileController extends EntryController implements ProviderSwitch {
        @Override
        public String getKey() {
            return "swapfile";
        }

        @Override
        protected MetaData getMetaData() {
            return new MetaData("com.android.settings.category.ia.system")
                    .setTitle("4GB Swap File")
                    .setSummary("4 GB swap file on internal storage (A performance drop may occur once swapping begins; ensure you have sufficient storage.)");
        }

        @Override
        public boolean isSwitchChecked() {
            return SystemProperties.getBoolean(PROP, true);
        }

        @Override
        public boolean onSwitchCheckedChanged(boolean checked) {
            SystemProperties.set(PROP, checked ? "1" : "0");
            return true;
        }

        @Override
        public String getSwitchErrorMessage(boolean attemptedChecked) {
            return "Failed";
        }
    }
}
