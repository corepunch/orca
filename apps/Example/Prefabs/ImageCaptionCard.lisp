(StackView Name="ImageCaptionCard" class="card stack-card image-caption-card"
  (ImageView Name="ImageCaptionCardImage" class="caption-image" Stretch="UniformToFill"
    Source=(bind "DataContext/Image"))
  (TextBlock Name="ImageCaptionCardTitle" class="caption-title" Text=(bind "DataContext/Title")))
