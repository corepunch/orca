#!/usr/bin/env python3
"""Compose panorama-led App Store artwork from real, offline Octopus captures.

Screens 1-3 unfold the home / connect / discover Pivot into one continuous
strip that runs behind the phones and across slot boundaries, so the carousel
itself reads as a sideways swipe. Header words outside a phone are pixel crops
of the app's own rendered headers, not re-typeset text. Phones always show
complete, unretouched captures.
"""
import hashlib
import json
from pathlib import Path
import zipfile
from PIL import Image, ImageDraw, ImageFont

ROOT = Path(__file__).resolve().parents[2]
OUTPUT = ROOT / 'marketing/octopus/app-store'
TYPEFACE = '/System/Library/Fonts/HelveticaNeue.ttc'
SIZES = [(1320, 2868), (1206, 2622)]
PAPER, INK, TEAL = '#faf9f6', '#111111', '#0e8f86'
CAPTURE = (1206, 2622)
# Capture rows (pixels): header word band, content area above the app bar.
WORD_TOP, WORD_BOTTOM, CONTENT_TOP, CONTENT_BOTTOM = 300, 480, 500, 2330
PIVOT = ['home', 'connect', 'discover']
SHOTS = {
    'home': ('home, connect,\ndiscover.', 'Swipe sideways through everything that matters to you.'),
    'connect': ("see who's\nlistening.", 'Follows, likes, reposts and replies in one place.'),
    'discover': ('find someone\nnew.', "Fresh voices from people you don't follow yet."),
    'post': ('every post,\na conversation.', 'Threaded replies under every post.'),
    'profile': ('get to know\nthe author.', 'Profiles gather posts and reposts together.'),
    'write': ('four ways\nto post.', 'Text, photos, quotes and links, with tags.'),
    'album': ('share up to\nnine photos.', 'Albums arrange themselves in balanced rows.'),
    'light': ("light or dark,\nit's yours.", 'Bold type and calm colour in either appearance.'),
}
TAGLINE = 'SWIPE SIDEWAYS.'


def font(size, weight='light'):
    return ImageFont.truetype(TYPEFACE, size, index={'regular': 0, 'bold': 1, 'light': 7}[weight])


def capture(shot):
    path = OUTPUT / shot['capture']
    if hashlib.sha256(path.read_bytes()).hexdigest() != shot['sha256']:
        raise ValueError('Capture changed since manifest: ' + str(path))
    image = Image.open(path).convert('RGB')
    assert image.size == CAPTURE, (path, image.size)
    return image


def phone(canvas, screen, x, y, width, outline=12):
    """Simple iPhone outline around the complete, unretouched capture."""
    height = round(screen.height * width / screen.width)
    draw = ImageDraw.Draw(canvas)
    radius = round(width * .106)
    for inset, color in [(outline+4, '#262626'), (outline+1, '#b7b7b7'), (outline-2, '#545454'), (5, '#080808')]:
        draw.rounded_rectangle((x-inset, y-inset, x+width+inset, y+height+inset),
                               radius=radius+inset, fill=color)
    mask = Image.new('L', (width, height))
    ImageDraw.Draw(mask).rounded_rectangle((0, 0, width-1, height-1), radius=radius, fill=255)
    canvas.paste(screen.resize((width, height), Image.Resampling.LANCZOS), (x, y), mask)
    return height


def selected_word(image):
    """Crop the selected (white) Pivot header word from a capture's header row."""
    band = image.crop((0, WORD_TOP, image.width, WORD_BOTTOM)).convert('L')
    pixels, right = band.load(), 0
    for x in range(band.width):
        if any(pixels[x, y] > 225 for y in range(0, band.height, 2)):
            right = x
    if right < 100:
        raise ValueError('No selected Pivot header found')
    return image.crop((0, WORD_TOP, min(image.width, right + 12), WORD_BOTTOM))


def pivot_strip(images, gap=44):
    """Unfold the three Pivot sections side by side at capture scale."""
    pitch = CAPTURE[0] + gap
    strip = Image.new('RGB', (pitch * len(PIVOT) - gap, CAPTURE[1]), 'black')
    for i, name in enumerate(PIVOT):
        content = images[name].crop((0, CONTENT_TOP, CAPTURE[0], CONTENT_BOTTOM))
        strip.paste(content, (i * pitch, CONTENT_TOP))
        strip.paste(selected_word(images[name]), (i * pitch, WORD_TOP))
    ImageDraw.Draw(strip).line((0, CONTENT_TOP - 2, strip.width, CONTENT_TOP - 2), fill='#2a2a2a', width=2)
    return strip, pitch


def headline(draw, title, subtitle):
    size = 116
    while max(draw.textlength(line, font=font(size)) for line in title.splitlines()) > 1144:
        size -= 1
    if size < 90:
        raise ValueError('Headline too small: ' + title)
    for i, line in enumerate(title.splitlines()):
        draw.text((82, 218 + i * 128), line, font=font(size), fill=INK)
    if draw.textlength(subtitle, font=font(34, 'regular')) > 1140:
        raise ValueError('Subtitle overflows: ' + subtitle)
    draw.text((90, 520), subtitle, font=font(34, 'regular'), fill='#555555')


def render(stage, index, total, images):
    width, height = SIZES[0]
    canvas = Image.new('RGB', (width, height), PAPER)
    draw = ImageDraw.Draw(canvas)
    draw.text((88, 88), 'OCTOPUS', font=font(34, 'bold'), fill=INK)
    draw.text((1232, 91), 'iPHONE', anchor='ra', font=font(28, 'regular'), fill='#555555')
    headline(draw, *SHOTS[stage])
    x, y, screen_width = 202, 682, 916
    scale = screen_width / CAPTURE[0]
    band_top, band_bottom = 845, 2500
    draw.rectangle((0, band_top, width, band_bottom), fill='black')
    visible_top = round((band_top - y) / scale)
    order = ['discover', 'post', 'profile', 'write', 'album', 'home']
    position = order.index(stage) if stage in order else len(order) - 1
    for direction in (-1, 1):
        name = order[(position + direction) % len(order)]
        neighbor(canvas, images[name], x + direction * round((CAPTURE[0] + 44) * scale), band_top, visible_top, scale)
    phone(canvas, images[stage], x, y, screen_width)
    draw = ImageDraw.Draw(canvas)
    draw.line((88, 2768, 1232, 2768), fill='#cececa', width=2)
    draw.text((88, 2794), TAGLINE, font=font(24, 'regular'), fill='#505050')
    draw.text((1232, 2794), f'{index+1:02} / {total:02}', anchor='ra', font=font(24, 'regular'), fill='#505050')
    return canvas


def neighbor(canvas, image, left, top, visible_top, scale):
    piece = image.crop((0, visible_top, image.width, CONTENT_BOTTOM))
    piece = piece.resize((round(piece.width * scale), round(piece.height * scale)), Image.Resampling.LANCZOS)
    canvas.paste(piece, (left, top))


def render_trio(images):
    """Screens 1-3 as one continuous canvas, cut into three slots.

    As in a Windows Phone Panorama, one oversized title runs across every
    section and past the right edge; each section's phone sits beneath it and
    bleeds off the bottom, so the store carousel reads as a sideways pan.
    """
    width, height = SIZES[0]
    wide = Image.new('RGB', (width * len(PIVOT), height), PAPER)
    draw = ImageDraw.Draw(wide)
    band_top = 640
    draw.rectangle((0, band_top, wide.width, height), fill='black')
    title = font(640)
    draw.text((70, 770 - title.getbbox('o')[1]), 'octopus social', font=title, fill='white')
    screen_width, y = 960, 1300
    for k, stage in enumerate(PIVOT):
        left = k * width
        draw.text((left + 88, 88), 'OCTOPUS', font=font(34, 'bold'), fill=INK)
        draw.text((left + 1232, 91), 'iPHONE', anchor='ra', font=font(28, 'regular'), fill='#555555')
        slot = Image.new('RGB', (width, band_top), PAPER)
        headline(ImageDraw.Draw(slot), *SHOTS[stage])
        wide.paste(slot.crop((0, 150, width, band_top)), (left, 150))
        phone(wide, images[stage], left + (width - screen_width) // 2, y, screen_width)
    # The pan continues into a post beyond the last section.
    scale = screen_width / CAPTURE[0]
    neighbor(wide, images['post'], wide.width - 120, y, 0, scale)
    return [wide.crop((k * width, 0, (k + 1) * width, height)) for k in range(len(PIVOT))]


def render_wide(images, strip):
    """Landscape review/campaign image of the unfolded Pivot plus a post."""
    canvas = Image.new('RGB', (4096, 1740), PAPER)
    draw = ImageDraw.Draw(canvas)
    draw.text((80, 66), 'OCTOPUS / iPHONE', font=font(38, 'bold'), fill=INK)
    draw.text((72, 145), 'swipe sideways.', font=font(162), fill=INK)
    draw.text((82, 346), 'Home, connect and discover, side by side.', font=font(43), fill='#555555')
    pieces = [strip, images['post'], images['album']]
    scale = 3936 / (sum(p.width for p in pieces) + 44 * (len(pieces) - 1) / 1)
    top, x = 470, 80.0
    draw.rectangle((0, top + round(220 * scale), 4096, top + round(CONTENT_BOTTOM * scale)), fill='black')
    for part in pieces:
        content = part.crop((0, 220, part.width, CONTENT_BOTTOM))
        size = (round(part.width * scale), round(content.height * scale))
        canvas.paste(content.resize(size, Image.Resampling.LANCZOS), (round(x), top + round(220 * scale)))
        x += part.width * scale + 44 * scale
    draw = ImageDraw.Draw(canvas)
    draw.line((80, 1690, 4016, 1690), fill='#cccccc', width=2)
    labels = [(80 + i * (CAPTURE[0] + 44) * scale, n) for i, n in enumerate(PIVOT + ['post', 'photos'])]
    for lx, label in labels:
        draw.text((round(lx), 1700), label, font=font(25), fill='#555555')
    canvas.save(OUTPUT / 'panorama-wide.png', optimize=True)
    canvas.resize((2048, 870), Image.Resampling.LANCZOS).save(OUTPUT / 'panorama-preview.jpg', quality=94)


def main():
    manifest = json.loads((OUTPUT / 'manifest.json').read_text())
    shots = manifest['shots']
    images = {shot['stage']: capture(shot) for shot in shots}
    strip, _ = pivot_strip(images)
    rendered, files = [], []
    trio = dict(zip(PIVOT, render_trio(images)))
    for i, shot in enumerate(shots):
        artwork = trio.get(shot['stage']) or render(shot['stage'], i, len(shots), images)
        rendered.append(artwork)
        for width, height in SIZES:
            directory = OUTPUT / f'en-GB/iphone-{width}x{height}'
            directory.mkdir(parents=True, exist_ok=True)
            target = directory / (shot['name'] + '.png')
            export = artwork if artwork.size == (width, height) else artwork.resize((width, height), Image.Resampling.LANCZOS)
            export.save(target, optimize=True)
            with Image.open(target) as check:
                assert check.mode == 'RGB' and check.size == (width, height)
            files.append(dict(path=str(target.relative_to(OUTPUT)), width=width, height=height,
                              sha256=hashlib.sha256(target.read_bytes()).hexdigest()))
    # The carousel as the store shows it: slots side by side, no gaps.
    thumb_w, thumb_h = 330, 717
    sheet = Image.new('RGB', (thumb_w * len(rendered), thumb_h), PAPER)
    for i, artwork in enumerate(rendered):
        sheet.paste(artwork.resize((thumb_w, thumb_h), Image.Resampling.LANCZOS), (i * thumb_w, 0))
    sheet.save(OUTPUT / 'contact-sheet.jpg', quality=95)
    render_wide(images, strip)
    manifest['exports'] = files
    manifest['captions'] = {stage: dict(title=t, subtitle=s) for stage, (t, s) in SHOTS.items()}
    (OUTPUT / 'manifest.json').write_text(json.dumps(manifest, indent=2) + '\n')
    with zipfile.ZipFile(OUTPUT / 'octopus-iphone-app-store-en-GB.zip', 'w', zipfile.ZIP_DEFLATED) as archive:
        for file in files:
            archive.write(OUTPUT / file['path'], file['path'])
    print(f'Verified {len(files)} RGB PNGs; exported contact sheet, panorama and upload ZIP.')


if __name__ == '__main__':
    main()
