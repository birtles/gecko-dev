/* Generated by make_unicode.py DO NOT MODIFY */
/* Unicode version: 12.0.0 */
#ifndef V8_JSREGEXPCHARACTERS_INL_H_
#define V8_JSREGEXPCHARACTERS_INL_H_

namespace js {

namespace irregexp {

static inline bool
RangeContainsLatin1Equivalents(CharacterRange range, bool unicode)
{
    if (unicode) {
        // "LATIN SMALL LETTER LONG S" case folds to "LATIN SMALL LETTER S".
        if (range.Contains(0x017F)) {
            return true;
        }
        // "LATIN CAPITAL LETTER SHARP S" case folds to "LATIN SMALL LETTER SHARP S".
        if (range.Contains(0x1E9E)) {
            return true;
        }
        // "KELVIN SIGN" case folds to "LATIN SMALL LETTER K".
        if (range.Contains(0x212A)) {
            return true;
        }
        // "ANGSTROM SIGN" case folds to "LATIN SMALL LETTER A WITH RING ABOVE".
        if (range.Contains(0x212B)) {
            return true;
        }
    }

    // "GREEK CAPITAL LETTER MU" case maps to "MICRO SIGN".
    // "GREEK SMALL LETTER MU" case maps to "MICRO SIGN".
    if (range.Contains(0x039C) || range.Contains(0x03BC)) {
        return true;
    }
    // "LATIN CAPITAL LETTER Y WITH DIAERESIS" case maps to "LATIN SMALL LETTER Y WITH DIAERESIS".
    if (range.Contains(0x0178)) {
        return true;
    }
    return false;
}

} } // namespace js::irregexp

#endif // V8_JSREGEXPCHARACTERS_INL_H_
