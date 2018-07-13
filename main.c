#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void str_replace(char *, const char *, const char *);

int main(int argc, char *argv[]){
   char *source = NULL;
   int rec = 1;

   if (argc == 1) {
      printf("\nusage: %s mindmap.mm\n\n", *argv);
      return 0;
   }

   char *arg1 = *argv;
   FILE *fp = fopen(*++argv, "r");
   if (fp != NULL) {
      fseek(fp, 0L, SEEK_END);
      long bufsize = ftell(fp);
      source = malloc(sizeof(char) * (bufsize));

      fseek(fp, 0L, SEEK_SET);
      fread(source, sizeof(char), bufsize, fp);
   }
   fclose(fp);
   //printf("%s", source);

   if ( argc == 2 ) {
      printf("<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?>\n\n");
      printf("<aiml>\n\n");
      printf("<meta name=\"author\" content=\"Alex Bylund\"/>\n");
      printf("<meta name=\"language\" content=\"en\"/>\n");
      rec = 0;
   }

   char *line, lineBuffer[10000], *tagtemp, *patternCaps;
   char *lineRead, *lineText, *lineLink;
   char tagstack[100][100];
   int depth = 0, count = 0;
   int nodeLine, endNode, singleNodeLine;

   char *tmp = strtok(source, "\n");
   while ( (line = strtok(NULL, "\n")) != NULL) {
      lineRead = NULL; lineRead = line;
      nodeLine = endNode = singleNodeLine = 0;

      if ( strstr(strdup(lineRead), "<node ") != NULL ) nodeLine = 1; // starting a node
      if ( strstr(strdup(lineRead), "</node>") != NULL ) endNode = 1; // ending a node with children
      if ( (strstr(strdup(lineRead), "/>") != NULL ) && nodeLine == 1 ) singleNodeLine = 1; //node with no children

      if (nodeLine) {
         lineText = strstr(strdup(lineRead), "TEXT=\"");
         lineLink = strstr(strdup(lineRead), "LINK=\"");

         if (lineText != NULL) {
            count = 0;
            lineText = strstr(lineText, "\""); lineText++;
            while (*lineText != '"') {lineBuffer[count] = *lineText; lineText++; count++;}
            lineBuffer[count] = '\0';

            tagtemp = strdup(lineBuffer);
            count = 0;
            while (*tagtemp != ' ' && *tagtemp != '\0') {tagstack[depth][count] = *tagtemp; tagtemp++; count++;}
            tagstack[depth][count] = '\0';

            /* should look for them all at once in one function
               but the strings are so small it might just be a
               problem with 3GB+ mind maps */
            str_replace(lineBuffer, "&quot;", "\"");
            str_replace(lineBuffer, "&apos;", "'");
            str_replace(lineBuffer, "&lt;", "<");
            str_replace(lineBuffer, "&gt;", ">");
            str_replace(lineBuffer, "&#xa;", "\n");
            str_replace(lineBuffer, "&amp;", "&");
         }
         if (!singleNodeLine && depth > 2) {
            printf("<%s>\n", lineBuffer);
            depth++;
         }
         if (!singleNodeLine && depth < 2) {
            if ( strstr(strdup(tagstack[depth]), "topic") != NULL ) {
               printf("\n<%s>", lineBuffer);
            }
            depth++; // move from main or organizing/topic node
            continue;
         }

         /* how a hack became a h4x0r1!111
            recursively system calls itself... since strtok has its own internal state
            note to future future proofing ^ */
         if (lineLink != NULL) {
            count = 0;
            lineLink = strstr(lineLink, "\""); lineLink++;
            while (*lineLink != '"') {lineBuffer[count] = *lineLink; lineLink++; count++;}
            lineBuffer[count] = '\0';

            char call[1000];
            strcat(call, arg1); strcat(call, " "); strcat(call, lineBuffer); strcat(call, " "); strcat(call, "recursion");
            fflush(stdout); system( call ); count = 0;

            while (*call != '\0') call[count]='\0',count++;
            nodeLine = endNode = singleNodeLine = 0;
         }

      }

      // top of the mind so print it normally
      if (singleNodeLine) {
         if (depth > 2) printf("%s\n", lineBuffer);
      }

      // write the closing tag for the closing node
      if (endNode) {
         if (depth > 3) printf("</%s>\n", tagstack[depth-1]);
         depth--;
      }

      if (depth == 2) { // first child nodes
         if (nodeLine) {

            // capitalize the pattern
            patternCaps = lineBuffer;
            while (*patternCaps != '\0') {
               if (*patternCaps > 96) *patternCaps = *patternCaps-32; patternCaps++;
               // ctype.h .. more portable?
               //*patternCaps = islower(*patternCaps) ? toupper(*patternCaps) : *patternCaps;
            }

            printf("\n<category>\n");
            printf("<pattern>%s</pattern>\n", lineBuffer);
            printf("<template>\n");
            if (!singleNodeLine) depth++;
         }
         if (endNode || singleNodeLine) {
            printf("</template>\n");
            printf("</category>\n");
         }
      }

      if (depth == 1) {
         if ( strstr(strdup(tagstack[depth]), "topic") != NULL )
            printf("</topic>\n");
      }

   }

   if ( !rec ){
      printf("\n</aiml>\n");
   }

   free(source);

   return 0;
}

void str_replace(char *target, const char *needle, const char *replacement) {
   // copy pasta!
   // https://stackoverflow.com/questions/32413667/replace-all-occurrences-of-a-substring-in-a-string-in-c
   char buffer[1024] = { 0 };
   char *insert_point = &buffer[0];
   const char *tmp = target;
   size_t needle_len = strlen(needle);
   size_t repl_len = strlen(replacement);
   while (1) {
      const char *p = strstr(tmp, needle);
      // walked past last occurrence of needle; copy remaining part
      if (p == NULL) {
         strcpy(insert_point, tmp);
         break;
      }
      // copy part before needle
      memcpy(insert_point, tmp, p - tmp);
      insert_point += p - tmp;
      // copy replacement string
      memcpy(insert_point, replacement, repl_len);
      insert_point += repl_len;
      // adjust pointers, move on
      tmp = p + needle_len;
  }
  // write altered string back to target
  strcpy(target, buffer);
}
