/* This sefun is meant to replace the deprecated efun tail().
 * Feel free to add it to your mudlibs, if you have much code using tail() or
 * want to use tail().
 */

#include <files.h>

#define TAIL_MAX_BYTES 1000

varargs int tail(string file)
{
    if (extern_call())
        set_this_object(previous_object());

    if (!stringp(file) || !this_player())
        return 0;
    bytes txt = read_bytes(file, -(TAIL_MAX_BYTES + 80), (TAIL_MAX_BYTES + 80));
    if (!bytesp(txt))
        return 0;

    // cut off first (incomplete) line
    int index = strstr(txt, b"\n");
    if (index > -1)
    {
        if (index + 1 < sizeof(txt))
            txt = txt[index+1..];
        else
            txt = b"";
    }

    tell_object(this_player(), to_text(txt, "ISO8859-1"));

    return 1;
}

