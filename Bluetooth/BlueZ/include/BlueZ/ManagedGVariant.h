#ifndef DEVICE_CLIENT_SDK_BLUETOOTH_BLUEZ_MANAGEDGVARIANT_H_
#define DEVICE_CLIENT_SDK_BLUETOOTH_BLUEZ_MANAGEDGVARIANT_H_

#include <memory>
#include <string>

#include <gio/gio.h>

namespace deviceClientSDK {
namespace bluetooth {
namespace blueZ {

class ManagedGVariant {
public:
    /**
     * Destructor.
     */
    ~ManagedGVariant();

    /**
     * Default constructor.
     */
    ManagedGVariant();

    /**
     * A constructor attaching to an existing @c GVariant. if @c variant is a floating reference,
     * it will be converted to normal one, extending its lifetime of the @c ManagedGVariant instance.
     */
    explicit ManagedGVariant(GVariant* variant);

    // Get a pointer to the internal @c GVariant variable.
    GVariant** asOutputParameter();

    // Return a raw @c GVariant* attached to this object.
    GVariant* get();

    // Method that dumps the contents of the wrapped variant to a string.
    std::string dumpToString(bool withAnnotations);

    // A helper method to extract variant contained in the variant attached to the object. This method is useful
    // when parsing variant containers where child elements could be boxed into the wrapper variant.
    ManagedGVariant unbox();
    
    // A helper conversion operator to allow checking if object has any variant attached.
    bool hasValue();
    
    // Swap the @c GVariant values with another instance of @c ManagedGVariant.
    void swap(ManagedGVariant& other);

private:
    // The variant attached.
    GVariant* m_variant;
};

inline ManagedGVariant::~ManagedGVariant() {
    if(m_variant) {
        g_variant_unref(m_variant);
    }
}

inline ManagedGVariant::ManagedGVariant() : m_variant{nullptr} {
}

inline ManagedGVariant::ManagedGVariant(GVariant* variant) : m_variant{variant} {
    if(variant && g_variant_is_floating(variant)) {
        g_variant_ref_sink(variant);
    }
}

inline GVariant** ManagedGVariant::asOutputParameter() {
    return &m_variant;
}

inline GVariant* ManagedGVariant::get() {
    return m_variant;
}

inline std::string ManagedGVariant::dumpToString(bool withAnnotations) {
    if(!m_variant) {
        return "<NULL>";
    }
    auto cstring = g_variant_print(m_variant, withAnnotations);
    std::string result = cstring;
    g_free(cstring);
    return result;
}

inline ManagedGVariant ManagedGVariant::unbox() {
    if(!m_variant) {
        return ManagedGVariant();
    }
    return ManagedGVariant(g_variant_get_variant(m_variant));
}

inline bool ManagedGVariant::hasValue() {
    return m_variant != nullptr;
}

inline void ManagedGVariant::swap(ManagedGVariant& other) {
    GVariant* temp = other.m_variant;
    other.m_variant = m_variant;
    m_variant = temp;
}

} // namespace blueZ
} // namespace bluetooth
} // namespace deviceClientSDK

#endif // DEVICE_CLIENT_SDK_BLUETOOTH_BLUEZ_MANAGEDGVARIANT_H_