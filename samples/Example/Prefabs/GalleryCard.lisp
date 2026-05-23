(StackView Name="GalleryCard" Direction="Vertical" Spacing=0 BackgroundColor="$card-background"
  (ImageView Name="GalleryCardImage" Height=180 Stretch="UniformToFill" Source="{../Card.Image}")
  (StackView Name="GalleryCardCaption" Direction="Vertical" Spacing=4 Padding=16
    (TextBlock Name="GalleryCardTitle" FontSize=15 ForegroundColor="$text-primary" Text="{../../Card.Title}")
    (TextBlock Name="GalleryCardBody" FontSize=12 ForegroundColor="$text-dim" WordWrap="true" Text="{../../Card.Subtitle}")))
