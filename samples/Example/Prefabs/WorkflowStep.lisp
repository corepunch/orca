(StackView Name="WorkflowStep" Direction="Vertical" Spacing=12 BackgroundColor="$card-background" Padding=24
  (TextBlock Name="WorkflowStepNumber" FontSize=32 Text="{../Card.Title}" ForegroundColor="{../Card.PrimaryColor}")
  (TextBlock Name="WorkflowStepTitle" FontSize=15 ForegroundColor="$text-primary" Text="{../Card.Subtitle}")
  (TextBlock Name="WorkflowStepBody" FontSize=12 ForegroundColor="$text-dim" WordWrap="true" Text="{../Card.Body}"))
