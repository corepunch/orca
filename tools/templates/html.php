<?php
// Generates HTML documentation from XML module files.
// Usage: php templates/html.php file1.xml [file2.xml ...] > docs/index.html

require __DIR__ . "/../model.php";

if ($argc < 2) {
    fwrite(STDERR, "Usage: php html.php file1.xml [file2.xml ...]\n");
    exit(1);
}

// Build combined workspace from all modules
$all_models   = [];
$ws_structs    = [];
$ws_enums      = [];
$ws_components = [];
$ws_resources  = [];

for ($i = 1; $i < $argc; $i++) {
    $m = new Model($argv[$i]);
    $all_models[] = $m;
    $ws_structs    = array_merge($ws_structs,    $m->structs);
    $ws_enums      = array_merge($ws_enums,      $m->enums);
    $ws_components = array_merge($ws_components, $m->components);
    $ws_resources  = array_merge($ws_resources,  $m->resources);
}

// ---------------------------------------------------------------------------
// DOM-based HTML builder
// ---------------------------------------------------------------------------

$dom = new DOMDocument('1.0', 'UTF-8');
$dom->formatOutput = true;

function el($tag, $attrs = [], $text = null) {
    global $dom;
    $node = $dom->createElement($tag);
    foreach ($attrs as $k => $v) {
        $node->setAttribute($k, $v);
    }
    if ($text !== null) {
        $node->appendChild($dom->createTextNode($text));
    }
    return $node;
}

function append_el($parent, $tag, $attrs = [], $text = null) {
    $node = el($tag, $attrs, $text);
    $parent->appendChild($node);
    return $node;
}

// Root HTML structure
$html = $dom->createElement('html');
$html->setAttribute('xmlns', 'http://www.w3.org/1999/xhtml');
$dom->appendChild($html);

$head = append_el($html, 'head');
append_el($head, 'meta', ['charset' => 'UTF-8']);
append_el($head, 'link', ['rel' => 'stylesheet', 'href' => 'doc.css']);
append_el($head, 'link', [
    'rel' => 'stylesheet',
    'href' => 'https://cdnjs.cloudflare.com/ajax/libs/font-awesome/6.5.2/css/all.min.css',
]);

$body = append_el($html, 'body');
$wrapper = append_el($body, 'div', [
    'class' => 'adjustable-sidebar-width full-width-container topic-wrapper',
]);
$sidebar = append_el($wrapper, 'div', ['class' => 'sidebar']);
$content = append_el($wrapper, 'main', ['class' => 'content']);

// ---------------------------------------------------------------------------
// Syntax highlighting tokenizer
// ---------------------------------------------------------------------------

$LUA_KEYWORDS = explode('|', 'require|and|break|do|else|elseif|end|false|for|function|goto|if|'
    . 'in|local|nil|not|or|repeat|return|then|true|until|while');

function highlight_lua($code) {
    global $dom, $LUA_KEYWORDS;
    $root = el('code', ['class' => 'source code-listing']);
    // Tokenize: strings, comments, words, numbers, symbols
    $pattern = '/"[^"\n]*"|--[^\n]*|\n|\t|\w+|\d+|[^\w\d\n\t]/';
    preg_match_all($pattern, $code, $matches);
    foreach ($matches[0] as $tok) {
        if (in_array($tok, $LUA_KEYWORDS)) {
            $root->appendChild(el('span', ['class' => 'syntax-keyword'], $tok));
        } elseif (strpos($tok, '--') !== false) {
            $root->appendChild(el('span', ['class' => 'syntax-comment'], $tok));
        } elseif (strpos($tok, '"') !== false) {
            $root->appendChild(el('span', ['class' => 'syntax-string'], $tok));
        } elseif (ctype_digit($tok)) {
            $root->appendChild(el('span', ['class' => 'syntax-number'], $tok));
        } elseif ($tok === "\n") {
            $root->appendChild($dom->createElement('br'));
        } elseif ($tok === "\t") {
            $root->appendChild(el('span', [], "\u{00A0}\u{00A0}\u{00A0}\u{00A0}"));
        } elseif (ctype_alnum($tok)) {
            $cls = ctype_upper($tok[0]) ? 'syntax-type' : 'syntax-variable';
            $root->appendChild(el('span', ['class' => $cls], $tok));
        } else {
            $root->appendChild(el('span', ['class' => 'syntax-identifier'], $tok));
        }
    }
    return $root;
}

function print_abstract($parent, $text) {
    global $dom;
    $parts = explode('`', $text);
    foreach ($parts as $i => $part) {
        if ($i % 2 === 1) {
            $parent->appendChild(el('code', ['class' => 'token-keyword'], $part));
        } else {
            $parent->appendChild($dom->createTextNode($part));
        }
    }
}

// ---------------------------------------------------------------------------
// Documentation element builders
// ---------------------------------------------------------------------------

function add_article($tag, $name, $parent_name, $ns) {
    global $content;
    return append_el($content, 'div', [
        'id'             => $name,
        'class'          => 'class',
        'data-name'      => $name,
        'data-inherits'  => $parent_name,
        'data-xmlns'     => $ns,
    ]);
}

function add_header($article, $type_label, $name, $brief) {
    append_el($article, 'div', ['class' => 'eyebrow'], $type_label);
    append_el($article, 'h1',  ['class' => 'header'],  $name);
    if ($brief) {
        append_el($article, 'div', ['class' => 'abstract'], $brief);
    }
}

function add_sidebar_link($icon_class, $name) {
    global $sidebar;
    $link = append_el($sidebar, 'a', ['class' => 'leaf-link', 'href' => '#' . $name]);
    append_el($link, 'i', ['class' => "fa-solid fa-{$icon_class} icon"]);
    append_el($link, 'span', [], $name);
}

function add_property_entry($article, $decor, $name, $type_str) {
    global $ws_enums, $ws_structs, $ws_components;
    $topic = append_el($article, 'div', ['class' => 'topic']);
    $code  = append_el($topic, 'code', ['class' => 'decorator']);
    append_el($code, 'span', [], "$decor ");
    $nm = append_el($code, 'span', ['class' => 'identifier'], $name);
    if ($type_str) {
        // Add ': type' suffix
        $nm->nodeValue = $name;
        $colon = $nm->ownerDocument->createTextNode(': ');
        $nm->parentNode->insertBefore($colon, $nm->nextSibling);
        if (isset($ws_enums[$type_str]) || isset($ws_structs[$type_str]) || isset($ws_components[$type_str])) {
            $a = el('a', ['class' => 'type-identifier-link', 'href' => '#' . $type_str], $type_str);
            $nm->parentNode->insertBefore($a, $nm->nextSibling->nextSibling);
        } else {
            $t = $nm->ownerDocument->createTextNode($type_str);
            $nm->parentNode->insertBefore($t, $nm->nextSibling->nextSibling);
        }
    }
    return $topic;
}

function add_small_abstract($article, $text) {
    if (!$text) return;
    $div = append_el($article, 'div', ['class' => 'small-abstract']);
    print_abstract($div, $text);
}

function add_function_entry($article, $method_elem, $returns_type_str) {
    global $dom;
    $name = lcfirst((string)$method_elem['name']);
    $topic = append_el($article, 'div', ['class' => 'topic']);
    $code  = append_el($topic, 'code', ['class' => 'decorator']);
    append_el($code, 'span', [], 'func ');
    $nm = append_el($code, 'span', ['class' => 'identifier'], $name);

    $params = iterator_to_array($method_elem->arg);
    if ($params) {
        $nm->nodeValue = $name;
        $nm->parentNode->insertBefore($dom->createTextNode('('), $nm->nextSibling);
        $last = count($params) - 1;
        foreach ($params as $pi => $arg) {
            $aname = append_el($code, 'span', ['class' => 'identifier'], (string)$arg['name']);
            $aname->parentNode->insertBefore($dom->createTextNode(':'), $aname->nextSibling);
            $atype = append_el($code, 'span', ['class' => 'decorator'], (string)$arg['type']);
            $atype->parentNode->insertBefore(
                $dom->createTextNode($pi < $last ? ', ' : ')'),
                $atype->nextSibling
            );
        }
    } else {
        $nm->nodeValue = $name;
        $nm->parentNode->insertBefore($dom->createTextNode('()'), $nm->nextSibling);
    }

    if ($returns_type_str !== null) {
        append_el($code, 'span', ['class' => 'decorator'], ' ->');
        append_el($code, 'span', [], ' ' . $returns_type_str);
    }

    $summary = (string)($method_elem->summary ?? '');
    add_small_abstract($topic, $summary ?: null);
    return $topic;
}

// ---------------------------------------------------------------------------
// Process each module
// ---------------------------------------------------------------------------

$tag_icons = [
    'struct'    => 'gear',
    'class'     => 'layer-group',
    'interface' => 'atom',
];

foreach ($all_models as $m) {
    $root_elem = $m->root;
    $ns = (string)($root_elem['namespace'] ?? '');
    $mod_name = $m->getModuleName();

    // Sidebar: technology title
    $tech = append_el($sidebar, 'div', ['class' => 'technology-title']);
    $i_el = append_el($tech, 'span', ['class' => 'fa-solid fa-folder-open icon']);
    $i_el->appendChild($dom->createTextNode("{$ns}.{$mod_name}"));

    // Enums
    foreach ($m->enums as $ename => $enum) {
        $article = add_article('div', $ename, 'None', 'None');
        add_header($article, 'Enum', $ename, (string)($enum->getElement()->summary ?? ''));
        add_sidebar_link('bars', $ename);
        append_el($article, 'h2', ['class' => 'title'], 'Constants');

        foreach ($enum->getValues() as $vname => $vdoc) {
            $topic = append_el($article, 'div', ['class' => 'topic']);
            $code  = append_el($topic, 'code', ['class' => 'decorator']);
            append_el($code, 'span', [], 'case ');
            append_el($code, 'span', ['class' => 'identifier'], $vname);
            add_small_abstract($topic, $vdoc ?: null);
        }
    }

    // Structs, interfaces, classes
    foreach ($m->structs as $sname => $struct) {
        $elem = $struct->getElement();
        $tag  = (string)$elem->getName();
        $brief = (string)($elem->summary ?? '');
        $parent_attr = (string)($elem['parent'] ?? 'None');

        $article = add_article('div', $sname, $parent_attr, (string)($elem->xmlns ?? 'None'));
        add_header($article, ucfirst($tag), $sname, $brief);
        add_sidebar_link($tag_icons[$tag] ?? 'gear', $sname);

        $fields = array_merge(
            iterator_to_array($elem->property, false),
            iterator_to_array($elem->field, false)
        );
        if (!empty($fields) || count($elem->method) > 0) {
            append_el($article, 'h2', ['class' => 'title'], 'Properties');
        }

        foreach ($fields as $field) {
            $fname = (string)$field['name'];
            $ftype = (string)$field['type'];
            $decor = 'var';
            if (isset($ws_enums[$ftype]))      $decor = 'enum';
            if (isset($ws_components[$ftype])) $decor = 'object';
            if (isset($ws_resources[$ftype]))  $decor = 'resource';

            if ((string)$field->getName() === 'field') {
                $topic = add_property_entry($article, $decor, $fname, $ftype);
                add_small_abstract($article, (string)$field ?: null);
            }
        }

        foreach ($elem->method as $method) {
            $returns = $method->returns ?? null;
            $ret_type = $returns ? (string)$returns['type'] : null;
            add_function_entry($article, $method, $ret_type);
        }
    }

    // Components
    foreach ($m->components as $cname => $component) {
        $elem = $component->getElement();
        $brief = (string)($elem->summary ?? '');
        $parent_attr = (string)($elem['parent'] ?? 'None');

        $article = add_article('div', $cname, $parent_attr, (string)($elem->xmlns ?? 'None'));
        add_header($article, 'Class', $cname, $brief);
        add_sidebar_link('layer-group', $cname);

        $props = $component->getProperties();
        if (!empty($props)) {
            append_el($article, 'h2', ['class' => 'title'], 'Properties');
        }

        foreach ($props as $pname => $ptype_obj) {
            $ptype = $ptype_obj->type;
            $decor = 'var';
            $struct = $ws_structs[$ptype] ?? null;
            if ($struct !== null) {
                $decor = 'struct';
            } elseif (isset($ws_enums[$ptype]) || isset($ws_enums[$pname])) {
                $decor = 'enum';
            } elseif (isset($ws_components[$ptype])) {
                $decor = 'object';
            }
            add_property_entry($article, $decor, $pname, $ptype);
        }

        foreach ($elem->method as $method) {
            $returns = $method->returns ?? null;
            $ret_type = $returns ? (string)$returns['type'] : null;
            add_function_entry($article, $method, $ret_type);
        }
    }
}

// ---------------------------------------------------------------------------
// Output HTML
// ---------------------------------------------------------------------------

echo $dom->saveHTML();
