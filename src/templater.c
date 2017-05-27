
// API:
// 		$tpl = new Templater($template_str);		- loads the template string
// 		$tpl->set("@macros", $value);				- set all found "@macros" with $value
// 		$tpl->render();								- return rendered template string

#include "php.h"
#include "templater.h"

zend_class_entry *templater_class;

PHPAPI char *php_str_to_str_ex(char *haystack, int length, 
char *needle, int needle_len, char *str, int str_len, int *_new_length, int case_sensitivity, int *replace_count)
{
    char *new_str;

    if (needle_len < length) {
        char *end, *haystack_dup = NULL, *needle_dup = NULL;
        char *e, *s, *p, *r;

        if (needle_len == str_len) {
            new_str = estrndup(haystack, length);
            *_new_length = length;

            if (case_sensitivity) {
                end = new_str + length;
                for (p = new_str; (r = php_memnstr(p, needle, needle_len, end)); p = r + needle_len) {
                    memcpy(r, str, str_len);
                    if (replace_count) {
                        (*replace_count)++;
                    }
                }
            } else {
                haystack_dup = estrndup(haystack, length);
                needle_dup = estrndup(needle, needle_len);
                php_strtolower(haystack_dup, length);
                php_strtolower(needle_dup, needle_len);
                end = haystack_dup + length;
                for (p = haystack_dup; (r = php_memnstr(p, needle_dup, needle_len, end)); p = r + needle_len) {
                    memcpy(new_str + (r - haystack_dup), str, str_len);
                    if (replace_count) {
                        (*replace_count)++;
                    }
                }
                efree(haystack_dup);
                efree(needle_dup);
            }
            return new_str;
        } else {
            if (!case_sensitivity) {
                haystack_dup = estrndup(haystack, length);
                needle_dup = estrndup(needle, needle_len);
                php_strtolower(haystack_dup, length);
                php_strtolower(needle_dup, needle_len);
            }

            if (str_len < needle_len) {
                new_str = emalloc(length + 1);
            } else {
                int count = 0;
                char *o, *n, *endp;

                if (case_sensitivity) {
                    o = haystack;
                    n = needle;
                } else {
                    o = haystack_dup;
                    n = needle_dup;
                }
                endp = o + length;

                while ((o = php_memnstr(o, n, needle_len, endp))) {
                    o += needle_len;
                    count++;
                }
                if (count == 0) {
                    //
                    if (haystack_dup) {
                        efree(haystack_dup);
                    }
                    if (needle_dup) {
                        efree(needle_dup);
                    }
                    new_str = estrndup(haystack, length);
                    if (_new_length) {
                        *_new_length = length;
                    }
                    return new_str;
                } else {
                    new_str = safe_emalloc(count, str_len - needle_len, length + 1);
                }
            }

            e = s = new_str;

            if (case_sensitivity) {
                end = haystack + length;
                for (p = haystack; (r = php_memnstr(p, needle, needle_len, end)); p = r + needle_len) {
                    memcpy(e, p, r - p);
                    e += r - p;
                    memcpy(e, str, str_len);
                    e += str_len;
                    if (replace_count) {
                        (*replace_count)++;
                    }
                }

                if (p < end) {
                    memcpy(e, p, end - p);
                    e += end - p;
                }
            } else {
                end = haystack_dup + length;

                for (p = haystack_dup; (r = php_memnstr(p, needle_dup, needle_len, end)); p = r + needle_len) {
                    memcpy(e, haystack + (p - haystack_dup), r - p);
                    e += r - p;
                    memcpy(e, str, str_len);
                    e += str_len;
                    if (replace_count) {
                        (*replace_count)++;
                    }
                }

                if (p < end) {
                    memcpy(e, haystack + (p - haystack_dup), end - p);
                    e += end - p;
                }
            }

            if (haystack_dup) {
                efree(haystack_dup);
            }
            if (needle_dup) {
                efree(needle_dup);
            }

            *e = '\0';
            *_new_length = e - s;

            new_str = erealloc(new_str, *_new_length + 1);
            return new_str;
        }
    } else if (needle_len > length) {
nothing_todo:
        *_new_length = length;
        new_str = estrndup(haystack, length);
        return new_str;
    } else {
        if (case_sensitivity && memcmp(haystack, needle, length)) {
            goto nothing_todo;
        } else if (!case_sensitivity) {
            char *l_haystack, *l_needle;

            l_haystack = estrndup(haystack, length);
            l_needle = estrndup(needle, length);

            php_strtolower(l_haystack, length);
            php_strtolower(l_needle, length);

            if (memcmp(l_haystack, l_needle, length)) {
                efree(l_haystack);
                efree(l_needle);
                goto nothing_todo;
            }
            efree(l_haystack);
            efree(l_needle);
        }

        *_new_length = str_len;
        new_str = estrndup(str, str_len);

        if (replace_count) {
            (*replace_count)++;
        }
        return new_str;
    }

}

PHP_METHOD(Templater, __construct)
{
    zval *__this;
    zval *template;

    if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_DC, "z", &template) == FAILURE)
    {
        RETURN_NULL();
    }

    __this = getThis();

    zend_update_property(Z_OBJCE_P(__this), __this, "_template", strlen("_template"), template TSRMLS_DC);
}

PHP_METHOD(Templater, set)
{
    char* needle;
    int needle_len;
    char* replace;
    int replace_len;
    
    if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_DC, "ss", &needle, &needle_len, &replace, &replace_len) == FAILURE)
    {
        WRONG_PARAM_COUNT;
    }

    zval *__this = getThis();
    zval *haystack;

    int case_sensitivity = 0;

    haystack = zend_read_property(Z_OBJCE_P(__this), __this, "_template", strlen("_template"), 1 TSRMLS_DC);		

    char* result;

    result = php_str_to_str_ex(Z_STRVAL_P(haystack), Z_STRLEN_P(haystack), needle, needle_len, replace, replace_len, &replace_len, case_sensitivity, NULL);

    zend_update_property_string(Z_OBJCE_P(__this), __this, "_template", strlen("_template"), result TSRMLS_DC);
}

PHP_METHOD(Templater, render)
{
    zval *__this;
    zval *template;

    __this 		= getThis();
    template 	= zend_read_property(Z_OBJCE_P(__this), __this, "_template", strlen("_template"), 1 TSRMLS_DC);

    RETURN_STRING(Z_STRVAL_P(template), 1);
}

static function_entry templater_functions[] = {
    PHP_ME(Templater, __construct, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(Templater, set, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(Templater, render, NULL, ZEND_ACC_PUBLIC)
    {NULL, NULL, NULL}
};

PHP_MINIT_FUNCTION(templater)
{
    zend_class_entry ce;

    INIT_CLASS_ENTRY(ce, "Templater", templater_functions);
    templater_class = zend_register_internal_class(&ce TSRMLS_DC);

    zend_declare_property_null(templater_class, "_template", strlen("_template"), ZEND_ACC_PRIVATE TSRMLS_DC);

    return SUCCESS;
}

zend_module_entry templater_module_entry = {
    STANDARD_MODULE_HEADER,
    "templater",
    templater_functions,
    PHP_MINIT(templater),
    NULL, NULL, NULL, NULL,
    "v 1.0", 
    STANDARD_MODULE_PROPERTIES
};    

ZEND_GET_MODULE(templater)
