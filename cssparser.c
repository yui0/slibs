/*
 * cssparser.c - Test/demo for cssparser.h
 *
 * Build:
 *   gcc -Wall -Wextra -o cssparser cssparser.c && ./cssparser
 *   gcc -Wall -Wextra -o cssparser cssparser.c && ./cssparser luna-ui.css
 */

#define CSS_PARSER_IMPLEMENTATION
#include "cssparser.h"

/* ── Callback demo ────────────────────────────────────────────────────── */

static void on_rule_start(void *ud) {
    (void)ud;
}

static void on_selector(void *ud, const CSSSelector *sel, int idx, int total) {
    (void)ud;
    if (idx == 0) printf("SELECTOR");
    /* Print one compound (the last one = target element) */
    if (sel->compound_count > 0) {
        int last = sel->compound_count - 1;
        const CSSCompound *cmp = &sel->compounds[last];
        if (cmp->part_count > 0) {
            const CSSSelectorPart *p = &cmp->parts[0];
            switch (p->type) {
            case CSS_SEL_TYPE:  printf(" type=%s", p->name); break;
            case CSS_SEL_CLASS: printf(" class=%s", p->name); break;
            case CSS_SEL_ID:    printf(" id=%s", p->name); break;
            default: break;
            }
        }
    }
    printf(" (spec=%06x)", sel->specificity);
    if (idx == total - 1) printf("\n");
    else printf(", ");
}

static void on_declaration(void *ud, const CSSDeclaration *decl) {
    (void)ud;
    printf("  DECL: %s = %s%s\n",
           decl->property, decl->value,
           decl->important ? " !important" : "");
}

static void on_rule_end(void *ud) {
    (void)ud;
    printf("\n");
}

/* ── Main ─────────────────────────────────────────────────────────────── */

int main(int argc, char *argv[]) {
    if (argc >= 2) {
        /* Parse CSS file and dump */
        CSSStyleSheet *sheet = css_parse_file(argv[1]);
        if (!sheet) { fprintf(stderr, "Failed to open: %s\n", argv[1]); return 1; }
        printf("Parsed %d rules, %d @rules\n", sheet->rule_count, sheet->at_rule_count);
        css_dump(sheet, stdout);
        css_free(sheet);
        return 0;
    }

    /* Built-in callback demo */
    const char *css =
        "@charset \"UTF-8\";\n"
        "@import url(\"base.css\");\n"
        "@keyframes progress-anim { to { width: 280px; } }\n"
        "@media (max-width: 768px) {\n"
        "  body { font-size: 14px; }\n"
        "  .window { border-radius: 10px !important; }\n"
        "}\n"
        "*, *::before, *::after { box-sizing: border-box; margin: 0; padding: 0; }\n"
        "body { background: radial-gradient(ellipse at 30% 20%, #312e6e 0%, #1a1a2e 100%);\n"
        "       font-family: -apple-system, BlinkMacSystemFont, sans-serif; }\n"
        "#main_win { width: 740px; height: 660px; left: 150px; top: 80px; }\n"
        ".window.focused { border-color: rgba(99,102,241,0.35); }\n"
        "a[href^=\"https\"] { color: green; }\n"
        "nav > ul + li ~ span { color: red; }\n"
        "h1, h2, h3 { font-weight: 700; color: #1e1e2e; }\n"
        ".btn:hover, .btn:focus-visible { background: #e0e0ec; }\n"
        ":root { --primary: #6366f1; --secondary: #a855f7; }\n";

    printf("=== Callback (SAX) mode ===\n\n");
    CSSCallbacks cb = {
        .user_data    = NULL,
        .on_rule_start = on_rule_start,
        .on_selector   = on_selector,
        .on_declaration= on_declaration,
        .on_rule_end   = on_rule_end,
    };
    css_parse_cb(css, 0, &cb);

    printf("\n=== DOM mode ===\n\n");
    CSSStyleSheet *sheet = css_parse(css, 0);
    printf("Rules: %d,  @rules: %d\n\n", sheet->rule_count, sheet->at_rule_count);
    css_dump(sheet, stdout);

    /* lookup test */
    if (sheet->rule_count > 0) {
        const CSSDeclaration *d = css_find_decl(&sheet->rules[0], "box-sizing");
        if (d) printf("\nLookup box-sizing: %s\n", d->value);
    }

    css_free(sheet);

    /* HTML integration test */
    printf("\n=== HTML integration ===\n\n");
    const char *html =
        "<!DOCTYPE html>\n"
        "<html>\n"
        "<head>\n"
        "<style>\n"
        "  body { color: red; }\n"
        "  .title { font-size: 24px; }\n"
        "</style>\n"
        "</head>\n"
        "<body>\n"
        "  <div style=\"background: blue; padding: 10px;\">\n"
        "    <p style=\"color: white; font-weight: bold;\">Hello</p>\n"
        "  </div>\n"
        "</body>\n"
        "</html>\n";

    CSSStyleSheet *html_sheet = css_parse_html(html, 0);
    if (html_sheet) {
        printf("HTML parsed: %d rules\n", html_sheet->rule_count);
        css_dump(html_sheet, stdout);
        css_free(html_sheet);
    }

    printf("\nDone.\n");
    return 0;
}
