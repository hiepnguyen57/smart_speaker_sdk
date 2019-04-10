#ifndef DEVICE_CLIENT_SDK_BLUETOOTHDEVICE_BLUEZ_GVARIANTTUPLEREADER_H_
#define DEVICE_CLIENT_SDK_BLUETOOTHDEVICE_BLUEZ_GVARIANTTUPLEREADER_H_

#include <functional>
#include <gio/gio.h>

#include "BlueZ/ManagedGVariant.h"
#include "BlueZ/ManagedGError.h"

namespace deviceClientSDK {
namespace bluetoothDevice {
namespace blueZ {

/**
 * Helper class to read values from a tuple represented by Glib's @c GVariant.
 * 
 * See <a href="https://developer.gnome.org/glib/stable/gvariant-format-strings.html">GVariant Format Strings</a>
 * for type strings explanation.
 */
class GVariantTupleReader {
public:
    /**
     * Destructor
     */
    ~GVariantTupleReader();

    /**
     * Constructor to initialize the reader with raw @c GVariant* value
     * 
     * @param originalVariant @c GVariant* value
     */
    explicit GVariantTupleReader(GVariant* originalVariant);

    /**
     * Constructor to initialize the reader with the reference to @c ManagedGVariant
     *
     * @param originalVariant A reference to @c ManagedGVariant
     */
    explicit GVariantTupleReader(ManagedGVariant& originalVariant);

    GVariantTupleReader(const GVariantTupleReader& other);

    // Get string value by index
    char* getCString(gsize index) const;

    // Get the object path by index.
    char* getObjectPath(gsize index) const;

    // Get the int32 value by index.
    gint32 getInt32(gsize index) const;

    // Get the boolean value by index.
    gboolean getBoolean(gsize index) const;

    // Get the @c Variant value by index adn return it
    ManagedGVariant getVariant(gsize index) const;

    //Returns a number of elements in a tuple.
    gsize size() const;

    // Helper function to traverse all the elements in a tuple
    bool forEach(std::function<bool(GVariant* value)> iteratorFunction) const;

private:
    /**
     * A @c GVariant* containing a tuple.
     */
    GVariant* m_tuple;
};

} // namespace blueZ
} // namespace bluetoothDevice
} // namespace deviceClientSDK

#endif // DEVICE_CLIENT_SDK_BLUETOOTH_BLUEZ_GVARIANTTUPLEREADER_H_