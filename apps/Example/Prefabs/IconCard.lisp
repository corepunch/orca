(StackView Name="IconCard" class="card stack-card icon-card"
  (StackView Name="IconCardHeader" class="icon-card-header"
    (StackView Name="IconCardBadge" class="icon-badge"
      BackgroundColor=(bind "DataContext/IconBackground")
      (VectorView Name="IconCardImage" class="icon-card-image" Stretch="Uniform"
        Source=(bind "DataContext/Icon")))
    (TextBlock Name="IconCardTitle" class="card-title" Text=(bind "DataContext/Title")))
  (TextBlock Name="IconCardBody" class="card-body" Text=(bind "DataContext/Body")))
