/*
 * Copyright (C) 2016 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef CAMERA_COMMON_1_0_VENDORTAGDESCRIPTOR_H
#define CAMERA_COMMON_1_0_VENDORTAGDESCRIPTOR_H

#include <system/camera_vendor_tags.h>
#include <utils/KeyedVector.h>
#include <utils/RefBase.h>
#include <utils/String8.h>
#include <utils/Vector.h>

#include <stdint.h>
#include <unordered_map>

namespace android {
namespace hardware {
namespace camera2 {
namespace params {

/**
 * VendorTagDescriptor objects are containers for the vendor tag
 * definitions provided, and are typically used to pass the vendor tag
 * information enumerated by the HAL to clients of the camera service.
 */
class VendorTagDescriptor {
  public:
    virtual ~VendorTagDescriptor();

    VendorTagDescriptor();
    VendorTagDescriptor(const VendorTagDescriptor& src);
    VendorTagDescriptor& operator=(const VendorTagDescriptor& rhs);

    void copyFrom(const VendorTagDescriptor& src);

    /**
     * The following 'get*' methods implement the corresponding
     * functions defined in
     * system/media/camera/include/system/camera_vendor_tags.h
     */

    // Returns the number of vendor tags defined.
    int getTagCount() const;

    // Returns an array containing the id's of vendor tags defined.
    void getTagArray(uint32_t* tagArray) const;

    // Returns the section name string for a given vendor tag id.
    const char* getSectionName(uint32_t tag) const;

    // Returns the index in section vectors returned in getAllSectionNames()
    // for a given vendor tag id. -1 if input tag does not exist.
    ssize_t getSectionIndex(uint32_t tag) const;

    // Returns the tag name string for a given vendor tag id.
    const char* getTagName(uint32_t tag) const;

    // Returns the tag type for a given vendor tag id.
    int getTagType(uint32_t tag) const;

    /**
     * Convenience method to get a vector containing all vendor tag
     * sections, or an empty vector if none are defined.
     * The pointer is valid for the lifetime of the VendorTagDescriptor,
     * or until copyFrom is invoked.
     */
    const SortedVector<String8>* getAllSectionNames() const;

    /**
     * Lookup the tag id for a given tag name and section.
     *
     * Returns OK on success, or a negative error code.
     */
    status_t lookupTag(const String8& name, const String8& section, /*out*/ uint32_t* tag) const;

    /**
     * Dump the currently configured vendor tags to a file descriptor.
     */
    void dump(int fd, int verbosity, int indentation) const;

  protected:
    KeyedVector<String8, KeyedVector<String8, uint32_t>*> mReverseMapping;
    KeyedVector<uint32_t, String8> mTagToNameMap;
    KeyedVector<uint32_t, uint32_t> mTagToSectionMap;  // Value is offset in mSections

    std::unordered_map<uint32_t, int32_t> mTagToTypeMap;
    SortedVector<String8> mSections;
    // must be int32_t to be compatible with Parcel::writeInt32
    int32_t mTagCount;

    vendor_tag_ops mVendorOps;
};
} /* namespace params */
} /* namespace camera2 */

namespace camera {
namespace common {
namespace helper {

/**
 * This version of VendorTagDescriptor must be stored in Android sp<>, and adds support for using it
 * as a global tag descriptor.
 *
 * It's a child class of the basic hardware::camera2::params::VendorTagDescriptor since basic
 * Parcelable objects cannot require being kept in an sp<> and still work with auto-generated AIDL
 * interface implementations.
 */
class VendorTagDescriptor : public ::android::hardware::camera2::params::VendorTagDescriptor,
                            public LightRefBase<VendorTagDescriptor> {
  public:
    /**
     * Create a VendorTagDescriptor object from the given vendor_tag_ops_t
     * struct.
     *
     * Returns OK on success, or a negative error code.
     */
    static status_t createDescriptorFromOps(const vendor_tag_ops_t* vOps,
                                            /*out*/
                                            sp<VendorTagDescriptor>& descriptor);

    /**
     * Sets the global vendor tag descriptor to use for this process.
     * Camera metadata operations that access vendor tags will use the
     * vendor tag definitions set this way.
     *
     * Returns OK on success, or a negative error code.
     */
    static status_t setAsGlobalVendorTagDescriptor(const sp<VendorTagDescriptor>& desc);

    /**
     * Returns the global vendor tag descriptor used by this process.
     * This will contain NULL if no vendor tags are defined.
     */
    static sp<VendorTagDescriptor> getGlobalVendorTagDescriptor();

    /**
     * Clears the global vendor tag descriptor used by this process.
     */
    static void clearGlobalVendorTagDescriptor();
};

} /* namespace helper */
} /* namespace common */
} /* namespace camera */

namespace camera2 {
namespace params {

class VendorTagDescriptorCache {
  public:
    typedef android::hardware::camera::common::helper::VendorTagDescriptor VendorTagDescriptor;
    VendorTagDescriptorCache(){};
    int32_t addVendorDescriptor(metadata_vendor_id_t id, sp<VendorTagDescriptor> desc);

    int32_t getVendorTagDescriptor(metadata_vendor_id_t id, sp<VendorTagDescriptor>* desc /*out*/);

    // Returns the number of vendor tags defined.
    int getTagCount(metadata_vendor_id_t id) const;

    // Returns an array containing the id's of vendor tags defined.
    void getTagArray(uint32_t* tagArray, metadata_vendor_id_t id) const;

    // Returns the section name string for a given vendor tag id.
    const char* getSectionName(uint32_t tag, metadata_vendor_id_t id) const;

    // Returns the tag name string for a given vendor tag id.
    const char* getTagName(uint32_t tag, metadata_vendor_id_t id) const;

    // Returns the tag type for a given vendor tag id.
    int getTagType(uint32_t tag, metadata_vendor_id_t id) const;

    /**
     * Dump the currently configured vendor tags to a file descriptor.
     */
    void dump(int fd, int verbosity, int indentation) const;

  protected:
    std::unordered_map<metadata_vendor_id_t, sp<VendorTagDescriptor>> mVendorMap;
    struct vendor_tag_cache_ops mVendorCacheOps;
};

} /* namespace params */
} /* namespace camera2 */

namespace camera {
namespace common {
namespace helper {

class VendorTagDescriptorCache
    : public ::android::hardware::camera2::params::VendorTagDescriptorCache,
      public LightRefBase<VendorTagDescriptorCache> {
  public:
    /**
     * Sets the global vendor tag descriptor cache to use for this process.
     * Camera metadata operations that access vendor tags will use the
     * vendor tag definitions set this way.
     *
     * Returns OK on success, or a negative error code.
     */
    static status_t setAsGlobalVendorTagCache(const sp<VendorTagDescriptorCache>& cache);

    /**
     * Returns the global vendor tag cache used by this process.
     * This will contain NULL if no vendor tags are defined.
     */
    static sp<VendorTagDescriptorCache> getGlobalVendorTagCache();

    /**
     * Clears the global vendor tag cache used by this process.
     */
    static void clearGlobalVendorTagCache();
};

}  // namespace helper

// NOTE: Deprecated namespace. This namespace should no longer be used for the following symbols
namespace V1_0::helper {
// Export symbols to the old namespace to preserve compatibility
typedef android::hardware::camera::common::helper::VendorTagDescriptor VendorTagDescriptor;
typedef android::hardware::camera::common::helper::VendorTagDescriptorCache
        VendorTagDescriptorCache;
}  // namespace V1_0::helper

}  // namespace common
}  // namespace camera
}  // namespace hardware
}  // namespace android

#endif /* CAMERA_COMMON_1_0_VENDORTAGDESCRIPTOR_H */
