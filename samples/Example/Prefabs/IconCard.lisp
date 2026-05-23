(StackView Name="IconCard" Direction="Vertical" Spacing=10 BackgroundColor="$card-background" Padding=28
  (StackView Name="IconCardHeader" Direction="Horizontal" Spacing=12 AlignItems="Center"
    (StackView Name="IconCardBadge" Width=48 Height=48 AlignItems="Center" JustifyContent="Center" BackgroundColor="{../../Card.IconBackground}"
      (ImageView Name="IconCardImage" Width=24 Height=24 Stretch="Uniform" Source="{../../../Card.Icon}" ForegroundColor="{../../../Card.PrimaryColor}"))
    (TextBlock Name="IconCardTitle" FontSize=16 ForegroundColor="$text-primary" Text="{../../Card.Title}"))
  (TextBlock Name="IconCardBody" FontSize=13 ForegroundColor="$text-dim" WordWrap="true" Text="{../Card.Body}"))
