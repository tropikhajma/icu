// © 2018 and later: Unicode, Inc. and others.
// License & terms of use: http://www.unicode.org/copyright.html

#include "unicode/utypes.h"

#if !UCONFIG_NO_FORMATTING && !UPRV_INCOMPLETE_CPP11_SUPPORT

// Allow implicit conversion from char16_t* to UnicodeString for this file:
// Helpful in toString methods and elsewhere.
#define UNISTR_FROM_STRING_EXPLICIT

#include "numparse_types.h"
#include "number_utypes.h"
#include "unicode/numberformatter.h"
#include "unicode/unumberformatter.h"
#include "fieldposutil.h"

using namespace icu;
using namespace icu::number;
using namespace icu::number::impl;


//////////////////////////////////
/// C API CONVERSION FUNCTIONS ///
//////////////////////////////////

UNumberFormatterData* UNumberFormatterData::validate(UNumberFormatter* input, UErrorCode& status) {
    auto* constInput = static_cast<const UNumberFormatter*>(input);
    auto* validated = validate(constInput, status);
    return const_cast<UNumberFormatterData*>(validated);
}

const UNumberFormatterData*
UNumberFormatterData::validate(const UNumberFormatter* input, UErrorCode& status) {
    if (U_FAILURE(status)) {
        return nullptr;
    }
    if (input == nullptr) {
        status = U_ILLEGAL_ARGUMENT_ERROR;
        return nullptr;
    }
    auto* impl = reinterpret_cast<const UNumberFormatterData*>(input);
    if (impl->fMagic != UNumberFormatterData::kMagic) {
        status = U_INVALID_FORMAT_ERROR;
        return nullptr;
    }
    return impl;
}

UNumberFormatter* UNumberFormatterData::exportForC() {
    return reinterpret_cast<UNumberFormatter*>(this);
}

UFormattedNumberData* UFormattedNumberData::validate(UFormattedNumber* input, UErrorCode& status) {
    auto* constInput = static_cast<const UFormattedNumber*>(input);
    auto* validated = validate(constInput, status);
    return const_cast<UFormattedNumberData*>(validated);
}

const UFormattedNumberData*
UFormattedNumberData::validate(const UFormattedNumber* input, UErrorCode& status) {
    if (U_FAILURE(status)) {
        return nullptr;
    }
    if (input == nullptr) {
        status = U_ILLEGAL_ARGUMENT_ERROR;
        return nullptr;
    }
    auto* impl = reinterpret_cast<const UFormattedNumberData*>(input);
    if (impl->fMagic != UFormattedNumberData::kMagic) {
        status = U_INVALID_FORMAT_ERROR;
        return nullptr;
    }
    return impl;
}

UFormattedNumber* UFormattedNumberData::exportForC() {
    return reinterpret_cast<UFormattedNumber*>(this);
}

/////////////////////////////////////
/// END CAPI CONVERSION FUNCTIONS ///
/////////////////////////////////////


U_CAPI UNumberFormatter* U_EXPORT2
unumf_openFromSkeletonAndLocale(const UChar* skeleton, int32_t skeletonLen, const char* locale,
                                UErrorCode* ec) {
    auto* impl = new UNumberFormatterData();
    if (impl == nullptr) {
        *ec = U_MEMORY_ALLOCATION_ERROR;
        return nullptr;
    }
    // Readonly-alias constructor (first argument is whether we are NUL-terminated)
    UnicodeString skeletonString(skeletonLen == -1, skeleton, skeletonLen);
    impl->fFormatter = NumberFormatter::fromSkeleton(skeletonString, *ec).locale(locale);
    return impl->exportForC();
}

U_CAPI UFormattedNumber* U_EXPORT2
unumf_openResult(UErrorCode* ec) {
    auto* impl = new UFormattedNumberData();
    if (impl == nullptr) {
        *ec = U_MEMORY_ALLOCATION_ERROR;
        return nullptr;
    }
    return impl->exportForC();
}

U_CAPI void U_EXPORT2
unumf_formatInt(const UNumberFormatter* uformatter, int64_t value, UFormattedNumber* uresult,
                UErrorCode* ec) {
    const UNumberFormatterData* formatter = UNumberFormatterData::validate(uformatter, *ec);
    UFormattedNumberData* result = UFormattedNumberData::validate(uresult, *ec);
    if (U_FAILURE(*ec)) { return; }

    result->string.clear();
    result->quantity.setToLong(value);
    formatter->fFormatter.formatImpl(result, *ec);
}

U_CAPI void U_EXPORT2
unumf_formatDouble(const UNumberFormatter* uformatter, double value, UFormattedNumber* uresult,
                   UErrorCode* ec) {
    const UNumberFormatterData* formatter = UNumberFormatterData::validate(uformatter, *ec);
    UFormattedNumberData* result = UFormattedNumberData::validate(uresult, *ec);
    if (U_FAILURE(*ec)) { return; }

    result->string.clear();
    result->quantity.setToDouble(value);
    formatter->fFormatter.formatImpl(result, *ec);
}

U_CAPI void U_EXPORT2
unumf_formatDecimal(const UNumberFormatter* uformatter, const char* value, int32_t valueLen,
                    UFormattedNumber* uresult, UErrorCode* ec) {
    const UNumberFormatterData* formatter = UNumberFormatterData::validate(uformatter, *ec);
    UFormattedNumberData* result = UFormattedNumberData::validate(uresult, *ec);
    if (U_FAILURE(*ec)) { return; }

    result->string.clear();
    result->quantity.setToDecNumber({value, valueLen}, *ec);
    if (U_FAILURE(*ec)) { return; }
    formatter->fFormatter.formatImpl(result, *ec);
}

U_CAPI int32_t U_EXPORT2
unumf_resultToString(const UFormattedNumber* uresult, UChar* buffer, int32_t bufferCapacity,
                     UErrorCode* ec) {
    const UFormattedNumberData* result = UFormattedNumberData::validate(uresult, *ec);
    if (U_FAILURE(*ec)) { return 0; }

    return result->string.toUnicodeString().extract(buffer, bufferCapacity, *ec);
}

U_CAPI void U_EXPORT2
unumf_resultGetField(const UFormattedNumber* uresult, UFieldPosition* ufpos, UErrorCode* ec) {
    if (ufpos == nullptr) {
        *ec = U_ILLEGAL_ARGUMENT_ERROR;
        return;
    }

    const UFormattedNumberData* result = UFormattedNumberData::validate(uresult, *ec);
    if (U_FAILURE(*ec)) { return; }

    UFieldPositionWrapper helper(*ufpos);
    result->string.populateFieldPosition(helper, 0, *ec);
}

U_CAPI void U_EXPORT2
unumf_resultGetAllFields(const UFormattedNumber* uresult, UFieldPositionIterator* ufpositer,
                         UErrorCode* ec) {
    if (ufpositer == nullptr) {
        *ec = U_ILLEGAL_ARGUMENT_ERROR;
        return;
    }

    const UFormattedNumberData* result = UFormattedNumberData::validate(uresult, *ec);
    if (U_FAILURE(*ec)) { return; }

    auto* helper = reinterpret_cast<FieldPositionIterator*>(ufpositer);
    result->string.populateFieldPositionIterator(*helper, *ec);
}

U_CAPI void U_EXPORT2
unumf_closeResult(const UFormattedNumber* uresult) {
    UErrorCode localStatus = U_ZERO_ERROR;
    const UFormattedNumberData* impl = UFormattedNumberData::validate(uresult, localStatus);
    if (U_FAILURE(localStatus)) { return; }
    delete impl;
}

U_CAPI void U_EXPORT2
unumf_close(UNumberFormatter* f) {
    UErrorCode localStatus = U_ZERO_ERROR;
    const UNumberFormatterData* impl = UNumberFormatterData::validate(f, localStatus);
    if (U_FAILURE(localStatus)) { return; }
    delete impl;
}


#endif /* #if !UCONFIG_NO_FORMATTING */



























