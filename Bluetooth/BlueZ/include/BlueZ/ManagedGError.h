#ifndef DEVICE_CLIENT_SDK_BLUETOOTH_BLUEZ_MANAGEDGERROR_H_
#define DEVICE_CLIENT_SDK_BLUETOOTH_BLUEZ_MANAGEDGERROR_H_

#include <string>

#include <gio/gio.h>

namespace deviceClientSDK {
namespace bluetooth {
namespace blueZ {

/**
 * Wrapper for GLib's @c GError returned by the most of DBus methods. This class is not thread safe.
 */
class ManagedGError {
public:
    /**
     * A constructor initializing object with existing @c GError* value.
     *
     * @param error error to be attached.
     */
    explicit ManagedGError(GError* error);

    /**
     * A constructor initializing object with an emtpy (no error) value
     */
    ManagedGError();

    /**
     * Check if object contains any error.
     *
     * @return true if there is an error
     */
    bool hasError();

    /**
     * Get a pointer to @c GError* variable
     *
     * @return A pointer to the internal variable holding the error
     */
    GError** toOutputParameter();

    /**
     * A destructor
     */
    ~ManagedGError();

    /**
     * Get the message associated with the error. Returns nullptr if there is no error. The pointer is valid as long
     * as the @c ManagedGError object exists.
     *
     * @return A message associated with the error.
     */
    const char* getMessage();

private:
    /**
     * A @c GError* value attached to the object or nullptr if there is no value
     */
    GError* m_error;
};

inline ManagedGError::ManagedGError(GError* error) : m_error{error} {
}

inline ManagedGError::ManagedGError() {
    m_error = nullptr;
}

inline bool ManagedGError::hasError() {
    return m_error != nullptr;
}

inline GError** ManagedGError::toOutputParameter() {
    return &m_error;
}

inline ManagedGError::~ManagedGError() {
    if (m_error != nullptr) {
        g_error_free(m_error);
    }
}

inline const char* ManagedGError::getMessage() {
    return m_error == nullptr ? nullptr : m_error->message;
}

} // namespace blueZ
} // namespace bluetooth
} // namespace deviceClientSDK

#endif // DEVICE_CLIENT_SDK_BLUETOOTH_BLUEZ_MANAGEDGERROR_H_
