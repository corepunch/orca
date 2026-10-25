(StackView Name="GalleryCard" class="card stack-card gallery-card"
  (ImageView Name="GalleryCardImage" class="gallery-image" Stretch="UniformToFill"
    Source=(bind "DataContext/Image") Node.Visible=(bool (bind "DataContext/Image")))
  (StackView Name="GalleryCardCaption" class="gallery-caption"
    (TextBlock Name="GalleryCardTitle" class="gallery-title" Text=(bind "DataContext/Title"))
    (TextBlock Name="GalleryCardBody" class="gallery-body" Text=(bind "DataContext/Subtitle"))))
