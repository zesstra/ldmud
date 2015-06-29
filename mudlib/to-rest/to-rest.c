#pragma strong_types,rtt_checks

#include <regexp.h>
#include <files.h>
#include <strings.h>

protected void create()
{
  seteuid(getuid());
}

mixed convert_file(string file)
{
  string rtext = read_file(file);
  string *sections = regexplode(rtext, "^[A-Ze\\h]+:*$", RE_PCRE|RE_MULTILINE);
  //return sections;
  string basename = explode(file,"/")[<1];

  if (basename[<4..] == ".rst")
    return 0;

  string out;
  // Titel schreiben
  switch(explode(file,"/")[2])
  {
    case "lfun":
    case "efun":
    case "sefun":
      out = basename + "()\n" + "="*sizeof(basename) + "==\n\n";
      break;
    default:
      out = basename + "\n" + "="*sizeof(basename) + "\n\n";
  }
  // Abschnitte schreiben
  foreach(string sec : sections)
  {
    //sec = regreplace(sec, "^[\\s]*", "", RE_PCRE);

    // Zeilenumbruch und endendes : abschneiden.
    sec = trim(sec, TRIM_BOTH, "\n");

    if (!sizeof(sec))
      continue;

    // ggf. alten titel wegwerfen
    if (sec == (basename+"()")
        || sec == basename )
      continue;

    // Zeilen aus "----" durch Leerzeilen ersetzen.
    sec = regreplace(sec, "^[\\-]*$", "", RE_PCRE|RE_MULTILINE);

    // Abschnittueberschrift?
    if (sec[0] >= 'A' && sec[0] <= 'Z')
    {
      // : entfernen.
      sec = regreplace(sec, ":", "", RE_PCRE);
      //sec = sec[0..<1];
      out += sec + "\n";
      // Abschnittsmarker und :: fuer nicht-interpretierten Block danach
      // schreiben
      out += "-" * sizeof(sec) + "\n" + "::\n\n";
    }
    // Abschnitt schreiben
    else
    {
      out += sec + "\n\n"; 
    }
  }
  //printf("%s\n",out);
  printf("Konvertiere %s\n", file);
  write_file(file+".rst", out, 1);
  return 1;
}

int convert_dir(string dir, int rec)
{
  mixed* dirs = get_dir(dir, GETDIR_NAMES|GETDIR_PATH);
  foreach(string file : dirs)
  {
    string base = explode(file, "/")[<1];
    if (base[0] == '.')
      continue;
    int size = file_size(file);
    if (size == FSIZE_DIR)
    {
      if (rec)
      {
        printf("Directory: %s\n",file+"/");
        convert_dir(file+"/", rec);
      }
    }
    else
    {
      convert_file(file);
    }
  }
  return 1;
}
