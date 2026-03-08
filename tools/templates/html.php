<?php require "model/module.php"; ?>
<?php
// Load all models from all provided XML files
$all_models = [];
$all_enums = [];
$all_structs = [];
$all_components = [];
$all_resources = [];

for ($i = 1; $i < $argc; $i++) {
    if (!file_exists($argv[$i])) continue;
    $m = new Model($argv[$i]);
    $all_models[] = $m;
    foreach ($m->getEnums() as $n => $v) $all_enums[$n] = $v;
    foreach ($m->getStructs() as $n => $v) $all_structs[$n] = $v;
    foreach ($m->getComponents() as $n => $v) $all_components[$n] = $v;
    foreach ($m->getResources() as $n => $v) $all_resources[$n] = $v;
}

function h($str) { return htmlspecialchars((string)$str, ENT_QUOTES, 'UTF-8'); }

function get_type_decor($type_name) {
    global $all_enums, $all_structs, $all_components, $all_resources;
    if (isset($all_enums[$type_name])) return 'enum';
    if (isset($all_components[$type_name])) return 'object';
    if (isset($all_resources[$type_name])) return 'resource';
    return 'var';
}

function render_property($name, $type_name, $decor) {
    global $all_enums, $all_structs, $all_components;
    $is_linked = isset($all_enums[$type_name]) || isset($all_structs[$type_name]) || isset($all_components[$type_name]);
    echo '<code class="decorator"><span>' . h($decor) . ' </span>';
    echo '<span class="identifier">' . h($name) . '</span>';
    if ($type_name) {
        echo ': ';
        if ($is_linked) {
            echo '<a class="type-identifier-link" href="#' . h($type_name) . '">' . h($type_name) . '</a>';
        } else {
            echo '<span>' . h($type_name) . '</span>';
        }
    }
    echo '</code>';
}

function render_function($method) {
    $name_str = lcfirst((string)$method->_elem["name"]);
    echo '<code class="decorator"><span>func </span>';
    echo '<span class="identifier">' . h($name_str) . '</span>';
    $args = $method->_elem->xpath("arg");
    if ($args) {
        $first = true;
        echo '(';
        foreach ($args as $arg) {
            if (!$first) echo ', ';
            echo '<span class="identifier">' . h($arg["name"]) . '</span>:';
            echo '<span class="decorator">' . h($arg["type"]) . '</span>';
            $first = false;
        }
        echo ')';
    } else {
        echo '()';
    }
    $returns = $method->_elem->xpath("returns");
    if ($returns) {
        echo '<span class="decorator"> -&gt;</span>';
        echo '<span> ' . h($returns[0]["type"]) . '</span>';
    }
    echo '</code>';
}

function render_small_abstract($text) {
    if (!$text) return;
    echo '<div class="small-abstract"><span>' . h(trim($text)) . "\n" . '</span></div>';
}

function render_article_header($id, $eyebrow, $name, $brief, $xmlns, $parent) {
    echo '<div id="' . h($id) . '" class="class" data-name="' . h($name) . '" data-inherits="' . h($parent ?: 'None') . '" data-xmlns="' . h($xmlns ?: 'None') . '">';
    echo '<div class="eyebrow">' . h($eyebrow) . "\n</div>";
    echo '<h1 class="header">' . h($name) . "\n</h1>";
    if ($brief) echo '<div class="abstract">' . h($brief) . "\n</div>";
}

function render_details($details_elem) {
    if (!$details_elem) return;
    $text = trim((string)$details_elem);
    if ($text) {
        echo '<h2 class="title">Overview</h2>';
        echo '<div class="abstract">' . h($text) . '</div>';
    }
}

function render_struct($model, $struct_elem, $tag = 'struct') {
    global $all_enums, $all_structs, $all_components, $all_resources;
    $sname = (string)$struct_elem["name"];
    $sbrief = (string)($struct_elem->summary ?? '');
    $sparent = (string)($struct_elem["parent"] ?? '');
    $sxmlns = (string)($struct_elem->xmlns ?? '');

    $eyebrow = ucfirst($tag);
    render_article_header($sname, $eyebrow, $sname, $sbrief, $sxmlns, $sparent);

    // Code block
    $module_name = (string)$model->getModuleName();
    echo '<code class="source"><span class="token-keyword">' . h($tag) . ' </span>';
    echo '<span class="token-identifier">' . h("{$module_name}.{$sname}") . '</span>';
    if ($sparent) {
        echo '<span class="token-keyword"> inherits </span>';
        echo '<ui class="comma-list"><li><a href="#' . h($sparent) . '" class="name base">' . h($sparent) . '</a></li></ui>';
    }
    echo '</code>';

    render_details($struct_elem->details);

    // Fields/properties
    $fields = $struct_elem->xpath("field") ?: [];
    $properties = $struct_elem->xpath("property") ?: [];
    if ($fields || $properties || $struct_elem->xpath("method")) {
        echo '<h2 class="title">Properties</h2>';
    }
    foreach (array_merge($properties, $fields) as $field) {
        $fname = (string)$field["name"];
        $ftype = (string)$field["type"];
        $decor = 'var';
        if (isset($all_enums[$ftype])) $decor = 'enum';
        if (isset($all_components[$ftype])) $decor = 'object';
        if (isset($all_resources[$ftype])) $decor = 'resource';
        if ($field->getName() === 'field') {
            echo '<div class="topic" style="margin-top:0">';
            render_property($fname, $ftype, $decor);
            echo '</div>' . "\n";
            render_small_abstract((string)$field);
        }
    }

    // Methods
    foreach ($struct_elem->xpath("method") ?: [] as $method_elem) {
        $method = new Method($method_elem, $model);
        $msummary = (string)($method_elem->summary ?? '');
        echo '<div class="topic">';
        render_function($method);
        render_small_abstract($msummary);
        echo '</div>';
    }

    echo '</div>';
}

function sidebar_link($href, $icon, $name) {
    echo '<a class="leaf-link" href="#' . h($href) . '">';
    echo '<i class="fa-solid fa-' . h($icon) . ' icon"></i>';
    echo '<span>' . h($name) . "\n</span>";
    echo '</a>';
}
?>
<html xmlns="http://www.w3.org/2001/XMLSchema"><head><link rel="stylesheet" href="doc.css"><link rel="stylesheet" href="https://cdnjs.cloudflare.com/ajax/libs/font-awesome/6.5.2/css/all.min.css"><meta charset="UTF-8"></head><body><div class="adjustable-sidebar-width full-width-container topic-wrapper"><div class="sidebar">
<?php foreach ($all_models as $model): ?>
<?php $ns = $model->getNamespace(); $mn = $model->getModuleName(); ?>
<div class="technology-title"><span class="fa-solid fa-folder-open icon"></span><?= h("{$ns}.{$mn}") . "\n" ?></div>
<?php foreach ($model->getEnums() as $ename => $enum): ?>
<?php sidebar_link($ename, 'bars', $ename); ?>
<?php endforeach ?>
<?php foreach ($model->getStructs() as $sname => $struct): ?>
<?php sidebar_link($sname, 'gear', $sname); ?>
<?php endforeach ?>
<?php foreach ($model->getComponents() as $cname => $component): ?>
<?php sidebar_link($cname, 'layer-group', $cname); ?>
<?php endforeach ?>
<?php endforeach ?>
</div><main class="content">
<?php foreach ($all_models as $model): ?>
<?php foreach ($model->getEnums() as $ename => $enum): ?>
<div id="<?= h($ename) ?>" class="class" data-name="<?= h($ename) ?>" data-inherits="None" data-xmlns="None"><div class="eyebrow">Enum
</div><h1 class="header"><?= h($ename) ?>
</h1><?php
$ebrief = (string)($enum->_elem->summary ?? '');
if ($ebrief) echo '<div class="abstract">' . h($ebrief) . "\n</div>";
render_details($enum->_elem->details);
echo '<h2 class="title">Constants</h2>';
foreach ($enum->getValues() as $vname => $vdoc): ?>
<div class="topic"><code class="decorator"><span>case </span><span class="identifier"><?= h($vname) ?></span></code><?php render_small_abstract($vdoc); ?></div>
<?php endforeach ?>
</div><?php endforeach ?>
<?php foreach ($model->getStructs() as $sname => $struct): ?>
<?php render_struct($model, $struct->_elem, 'struct'); ?>
<?php endforeach ?>
<?php foreach ($model->getComponents() as $cname => $component): ?>
<?php render_struct($model, $component->_elem, 'class'); ?>
<?php endforeach ?>
<?php endforeach ?>
</main></div></body></html>
